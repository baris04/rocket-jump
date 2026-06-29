import os
import unreal


OUT = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()), "DumpFirstPersonBlueprint.txt")


def line(text):
    unreal.log("[DumpFirstPersonBlueprint] " + text)
    with open(OUT, "a", encoding="utf-8") as handle:
        handle.write(text + "\n")


with open(OUT, "w", encoding="utf-8") as handle:
    handle.write("")

bp = unreal.EditorAssetLibrary.load_asset("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter")
if not bp:
    raise RuntimeError("Missing BP_FirstPersonCharacter")

subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
library = unreal.SubobjectDataBlueprintFunctionLibrary
handles = subsystem.k2_gather_subobject_data_for_blueprint(bp)
line("handle_count=" + str(len(handles)))

for index, handle in enumerate(handles):
    data = library.get_data(handle)
    obj = library.get_associated_object(data)
    try:
        obj_bp = library.get_object_for_blueprint(data, bp)
    except Exception:
        obj_bp = None
    assoc = library.get_associated_object(data)
    cls = obj.get_class() if obj else None
    parent = library.get_parent_handle(data)
    line(
        str(index)
        + " display="
        + str(library.get_display_name(data))
        + " variable="
        + str(library.get_variable_name(data))
        + " class="
        + (cls.get_name() if cls else "None")
        + " obj="
        + (obj.get_name() if obj else "None")
        + " bp_obj="
        + (obj_bp.get_name() if obj_bp else "None")
        + " assoc="
        + (assoc.get_name() if assoc else "None")
        + " is_component="
        + str(library.is_component(data))
        + " is_scene="
        + str(library.is_scene_component(data))
        + " is_root="
        + str(library.is_root_component(data))
        + " parent_valid="
        + str(library.is_handle_valid(parent))
    )

line("OUT=" + OUT)
