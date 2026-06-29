import os
import unreal


CHARACTER_BP_PATH = "/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"
ROCKET_COMPONENT_BP_PATH = "/Game/Blueprints/Components/BP_RocketJumpComponent"
WEAPON_MESH_PATH = "/Game/Weapons/GrenadeLauncher/Meshes/SM_GrenadeLauncher.SM_GrenadeLauncher"
OUT = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()), "SetupFirstPersonBlueprint.txt")


def line(text):
    unreal.log("[SetupFirstPersonBlueprint] " + text)
    with open(OUT, "a", encoding="utf-8") as handle:
        handle.write(text + "\n")


def fail(message):
    line("ERROR: " + message)
    raise RuntimeError(message)


with open(OUT, "w", encoding="utf-8") as handle:
    handle.write("")

bp = unreal.EditorAssetLibrary.load_asset(CHARACTER_BP_PATH)
if not bp:
    fail("Missing " + CHARACTER_BP_PATH)

rocket_component_class = unreal.EditorAssetLibrary.load_blueprint_class(ROCKET_COMPONENT_BP_PATH)
if not rocket_component_class:
    fail("Missing " + ROCKET_COMPONENT_BP_PATH)

weapon_mesh = unreal.EditorAssetLibrary.load_asset(WEAPON_MESH_PATH)
if not weapon_mesh:
    weapon_mesh = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
if not weapon_mesh:
    fail("Missing weapon mesh fallback")

subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
library = unreal.SubobjectDataBlueprintFunctionLibrary


def gather():
    return subsystem.k2_gather_subobject_data_for_blueprint(bp)


def data_for(handle):
    return library.get_data(handle)


def object_for(handle):
    return library.get_associated_object(data_for(handle))


def variable_for(handle):
    return str(library.get_variable_name(data_for(handle)))


def display_for(handle):
    return str(library.get_display_name(data_for(handle)))


def find_handle_by_variable(variable_name):
    for handle in gather():
        if variable_for(handle) == variable_name:
            return handle
    return None


def find_handle_by_class_name(class_name):
    for handle in gather():
        obj = object_for(handle)
        if obj and obj.get_class().get_name() == class_name:
            return handle
    return None


def add_subobject(parent_handle, new_class, variable_name):
    params = unreal.AddNewSubobjectParams()
    params.set_editor_property("parent_handle", parent_handle)
    params.set_editor_property("new_class", new_class)
    params.set_editor_property("blueprint_context", bp)

    result = subsystem.add_new_subobject(params)
    if isinstance(result, tuple):
        new_handle = result[0]
        fail_reason = result[1] if len(result) > 1 else ""
    else:
        new_handle = result
        fail_reason = ""

    if not library.is_handle_valid(new_handle):
        fail("Could not add " + variable_name + ": " + str(fail_reason))

    try:
        subsystem.rename_subobject(new_handle, unreal.Text(variable_name))
    except Exception:
        try:
            subsystem.rename_subobject_member_variable(new_handle, unreal.Name(variable_name))
        except Exception as error:
            line("Rename warning for " + variable_name + ": " + str(error))

    return new_handle


def configure_weapon_component(component):
    component.set_static_mesh(weapon_mesh)
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    component.set_cast_shadow(False)
    component.set_only_owner_see(True)
    component.set_visibility(True)
    component.set_hidden_in_game(False)
    component.set_editor_property("relative_location", unreal.Vector(30.0, 14.0, -16.0))
    component.set_editor_property("relative_rotation", unreal.Rotator(0.0, -90.0, 0.0))
    component.set_editor_property("relative_scale3d", unreal.Vector(0.4, 0.4, 0.4))

    if hasattr(component, "set_editor_property"):
        try:
            component.set_editor_property("depth_priority_group", unreal.SceneDepthPriorityGroup.SDPG_FOREGROUND)
        except Exception:
            pass


actor_handle = gather()[0]
camera_handle = find_handle_by_variable("FirstPersonCamera")
if not camera_handle:
    fail("Missing FirstPersonCamera component")

rocket_handle = find_handle_by_variable("RocketJump")
if not rocket_handle:
    rocket_handle = add_subobject(actor_handle, rocket_component_class, "RocketJump")
    line("Added RocketJump Blueprint component")
else:
    line("RocketJump component already exists")

weapon_handle = find_handle_by_variable("RocketJumpGunMesh")
if not weapon_handle:
    weapon_handle = add_subobject(camera_handle, unreal.StaticMeshComponent, "RocketJumpGunMesh")
    line("Added RocketJumpGunMesh component")
else:
    line("RocketJumpGunMesh component already exists")

weapon_component = object_for(weapon_handle)
if weapon_component:
    configure_weapon_component(weapon_component)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
unreal.EditorAssetLibrary.save_asset(CHARACTER_BP_PATH, only_if_is_dirty=False)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
line("Finished setup for " + CHARACTER_BP_PATH)
line("OUT=" + OUT)
