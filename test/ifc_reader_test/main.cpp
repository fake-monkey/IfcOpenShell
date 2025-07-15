#include "../ifcgeom/Iterator.h"
#include "../ifcgeom/kernels/opencascade/OpenCascadeConversionResult.h"
#include "../ifcparse/Ifc2x3.h"
#include "../ifcparse/Ifc4.h"
#include "../ifcparse/IfcFile.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <filesystem>
#include <format>
#include <Interface_Static.hxx>
#include <ranges>
#include <STEPControl_Writer.hxx>
#include <thread>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <vector>

TopoDS_Shape ApplyTransformation(const TopoDS_Shape& s, const gp_Trsf& t) {
    if (t.Form() == gp_Identity) {
        return s;
    } else {
        /// @todo set to 1. and exactly 1. or use epsilon?
        if (t.ScaleFactor() != 1.) {
            return BRepBuilderAPI_Transform(s, t, true);
        } else {
            return s.Moved(t);
        }
    }
}

namespace ranges_handle {
template <std::ranges::range TContainer>
std::vector<std::ranges::range_value_t<TContainer>> ConvertToVector(TContainer&& a_container) {
    return std::vector<std::ranges::range_value_t<TContainer>>(
        std::ranges::begin(a_container), std::ranges::end(a_container));
}
} // namespace ranges_handle

enum class IfcParseError {
    kIfcInitializationFailed,
    kIfcIteratorInitializationFailed,
    kIfcParsingException,
};

static std::optional<int> GetComponentCount(IfcParse::IfcFile& a_file) {
    if (auto product_2x3 = a_file.instances_by_type<Ifc2x3::IfcProduct>()) {
        return product_2x3->size();
    }
    if (auto product_4 = a_file.instances_by_type<Ifc4::IfcProduct>()) {
        return product_4->size();
    }
    return std::nullopt;
}

template <size_t TSize>
void SetDefaultOption(ifcopenshell::geometry::Settings& a_setting) {
    using TTupleType = typename ifcopenshell::geometry::Settings::settings_tuple;
    using TDerived = typename std::tuple_element_t<TSize, TTupleType>;
    auto& tuple_item = a_setting.get<TDerived>();
    if constexpr (ifcopenshell::geometry::HasDefault<TDerived>()) {
        tuple_item.value = tuple_item.defaultvalue;
    }

    if constexpr (TSize + 1 < std::tuple_size_v<TTupleType>) {
        SetDefaultOption<TSize + 1>(a_setting);
    }
}

// 返回类型为 std::variant，包含成功的形状向量或错误代码
static std::variant<std::vector<std::tuple<int, TopoDS_Shape>>, IfcParseError> ReadIFCShapes(std::istream& a_stream,
                                                                                             std::streamsize a_length,
                                                                                             unsigned int a_num_threads,
                                                                                             const std::set<std::string>& a_exclude_entities) {
    try {
        using namespace ifcopenshell;
        using namespace ifcopenshell::geometry;
        using namespace boost;

        std::unique_ptr<IfcParse::IfcFile> ifc_file = std::make_unique<IfcParse::IfcFile>(a_stream, (int)a_length);
        if (!(bool)ifc_file || !ifc_file->good()) {
            return IfcParseError::kIfcInitializationFailed;
        }

        auto count_model = GetComponentCount(*ifc_file);

        std::string geometry_kernel = "opencascade";
        geometry::Settings geometry_settings;

        SetDefaultOption<0>(geometry_settings);

        // 来自 ifcConvert 向 stp 转化过程。
        geometry_settings.get<settings::WeldVertices>().value = false;
        geometry_settings.get<settings::UseWorldCoords>().value = true;
        geometry_settings.get<settings::IteratorOutput>().value = settings::NATIVE;
        geometry_settings.get<settings::ConvertBackUnits>().value = true;

        IfcGeom::entity_filter entity_filter;
        entity_filter.include = false;
        entity_filter.traverse = true;
        if (a_exclude_entities.empty()) {
            // 来自 ifcConvert 向 stp 转化过程。
            entity_filter.entity_names.insert("IfcSpace");
            entity_filter.entity_names.insert("IfcOpeningElement");
        } else {
            entity_filter.entity_names = a_exclude_entities;
        }

        std::vector<IfcGeom::filter_t> filter_funcs;
        filter_funcs.push_back(boost::ref(entity_filter));

        int num_threads = a_num_threads;
        std::unique_ptr<IfcGeom::Iterator> context_iterator = std::make_unique<IfcGeom::Iterator>(
            geometry_kernel, geometry_settings, ifc_file.get(), filter_funcs, num_threads);

        if (!(bool)context_iterator || !context_iterator->initialize()) {
            return IfcParseError::kIfcIteratorInitializationFailed;
        }

        std::vector<std::tuple<int, TopoDS_Shape>> result;
        int cur_size = 1;
        while (true) {
            IfcGeom::Element* geom_object = context_iterator->get();
            int id = geom_object->id();

            // undone
            if (id == 1836) {
                std::cout << "\n";
            }
            // end

            if (geom_object != nullptr) {
                auto* o = static_cast<const IfcGeom::BRepElement*>(geom_object);
                std::unique_ptr<IfcGeom::ConversionResultShape> itm(o->geometry().as_compound(true));
                TopoDS_Shape compound = ((geometry::OpenCascadeShape*)itm.get())->shape();
                //if (o->id() == 62)
                {
                    //ShapeHandle::CreateSolidsFromSingleShape(compound, 1e-3, true);
                    gp_Trsf scale_trsf;
                    scale_trsf.SetScaleFactor(1000);
                    compound = ApplyTransformation(compound, scale_trsf);
                    result.push_back({id, compound});
                }
            }

            cur_size++;
            if (!context_iterator->next()) {
                break;
            }
        }
        if (num_threads > 1) {
            std::ranges::sort(result, std::less<int>{}, [](const auto& a_pair) {
                return std::get<0>(a_pair);
            });
        }
        return result;
        /*
        return ranges_handle::ConvertToVector(result | std::views::transform(
                                                           [](const auto& a_pair) {
                                                               return std::get<1>(a_pair);
                                                           }));
        */
    } catch (...) {
        return IfcParseError::kIfcParsingException;
    }
}

int main() {
    using namespace std;

    std::locale::global(std::locale("zh_CN.UTF-8"));

    filesystem::path ifc_dir = LR"(D:\works\tasks\BUGFIX#78415-BUGFIX#78907-ifc导入错误)";
    ifc_dir = LR"(D:\works\tasks\BUGFIX#78415-BUGFIX#78907-ifc导入错误\读取模型错误)";
    std::string file_name = "1GLF14-2(1)";
    filesystem::path ifc_path = ifc_dir / (file_name + ".ifc");
    uint32_t n = std::thread::hardware_concurrency();
    if (n == 0) {
        n = 1;
    }

    std::unique_ptr<std::ifstream> file_stream =
        std::make_unique<std::ifstream>(ifc_path, std::ios::binary | std::ios::ate);
    if (!file_stream->good()) {
        return {};
    }

    std::streamsize length = file_stream->tellg();
    file_stream->seekg(0, std::ios::beg);
    std::set<std::string> exclude_entities = {"IFCMECHANICALFASTENER", "IFCFASTENER"}; // Example of entities to exclude
    auto res = ReadIFCShapes(*file_stream, length, n, exclude_entities);

    if (auto* shapes = std::get_if<std::vector<std::tuple<int, TopoDS_Shape>>>(&res)) {

        STEPControl_Writer writer;
        for (const auto& [id, shape] : *shapes) {
            Interface_Static::SetCVal("write.step.product.name", std::format("{}", id).c_str());
            writer.Transfer(shape, STEPControl_AsIs);
        }

        filesystem::path out_path = ifc_dir / std::format("{}_debug.stp", file_name);
        if (writer.Write(out_path.string().c_str()) != IFSelect_RetDone) {
            std::cerr << "Save error!";
        };
    }

    return 0;
}
