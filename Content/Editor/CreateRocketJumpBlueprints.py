import os

import unreal


ASSET_ROOT = "/Game/Blueprints"
CONTENT_ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir())

BLUEPRINT_SPECS = [
    ("Core", "BP_RocketJumpGameMode", unreal.GameModeBase, "Owns the project defaults for pawn, controller, and HUD."),
    ("Core", "BP_RocketJumpCharacter", unreal.Character, "Fallback player character when the FirstPerson template character is unavailable."),
    ("Core", "BP_RocketJumpPlayerController", unreal.PlayerController, "Player input and possession controller."),
    ("Core", "BP_RocketJumpHUD", unreal.HUD, "Draws health and damage feedback."),
    ("Components", "BP_RocketJumpComponent", unreal.ActorComponent, "Rocket jump input, firing, recoil, and launch behavior."),
    ("Components", "BP_RocketJumpHealthComponent", unreal.ActorComponent, "Health, damage, death, respawn, and regeneration behavior."),
    ("Components", "BP_RotatingPlatformComponent", unreal.ActorComponent, "Rotates platforms around selected axes."),
    ("Gameplay", "BP_RocketJumpProjectile", unreal.Actor, "Player rocket projectile that explodes and launches the player."),
    ("Gameplay", "BP_RocketJumpTurret", unreal.Actor, "Enemy turret that tracks the player and fires projectiles."),
    ("Gameplay", "BP_TurretProjectile", unreal.Actor, "Projectile fired by turrets that damages the player."),
    ("Gameplay", "BP_RocketJumpPickup", unreal.Actor, "Pickup that grants double jump and can respawn."),
    ("Gameplay", "BP_RocketJumpLaserBeam", unreal.Actor, "Hazard beam that damages overlapping players."),
    ("World", "BP_LevelSetup", unreal.Actor, "Places level hazards and platform helpers at runtime."),
    ("World", "BP_ShowcaseSetup", unreal.Actor, "Builds a quick showcase parkour level at runtime."),
]


def log(message):
    unreal.log("[RocketJumpBlueprints] " + message)


def asset_path_to_filename(asset_path):
    if not asset_path.startswith("/Game/"):
        raise ValueError("Only /Game assets are supported: " + asset_path)

    relative_path = asset_path[len("/Game/"):] + ".uasset"
    return os.path.join(CONTENT_ROOT, relative_path.replace("/", os.sep))


def save_asset_or_fail(asset_path):
    saved = unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
    if not saved:
        raise RuntimeError("Unreal failed to save asset: " + asset_path)


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def make_blueprint_factory(parent_class):
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    return factory


def create_or_load_blueprint(asset_name, package_path, parent_class):
    asset_path = package_path + "/" + asset_name
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        existing_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        log("Exists: " + asset_path)
        return existing_asset

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    blueprint = asset_tools.create_asset(
        asset_name,
        package_path,
        None,
        make_blueprint_factory(parent_class),
    )

    if not blueprint:
        raise RuntimeError("Could not create Blueprint: " + asset_path)

    save_asset_or_fail(asset_path)
    log("Created: " + asset_path)
    return blueprint


def recreate_blueprint_if_needed(asset_name, package_path, parent_class):
    asset_path = package_path + "/" + asset_name
    return create_or_load_blueprint(asset_name, package_path, parent_class)


def load_blueprint_class(asset_path):
    loaded_class = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
    if not loaded_class:
        raise RuntimeError("Could not load generated class for " + asset_path)
    return loaded_class


def load_optional_blueprint_class(asset_path):
    loaded_class = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
    if loaded_class:
        log("Using Blueprint class: " + asset_path)
    else:
        log("Missing Blueprint class, using fallback later: " + asset_path)
    return loaded_class


def try_get_editor_property(obj, property_name):
    try:
        return obj.get_editor_property(property_name)
    except Exception:
        return None


def try_set_editor_property(obj, property_name, value):
    try:
        obj.set_editor_property(property_name, value)
        return True
    except Exception as error:
        log("Could not set " + property_name + ": " + str(error))
        return False


def first_existing_asset(asset_paths):
    for asset_path in asset_paths:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset:
            return asset
    return None


def configure_playable_default_pawn():
    character_class = load_blueprint_class(ASSET_ROOT + "/Core/BP_RocketJumpCharacter")
    character_cdo = unreal.get_default_object(character_class)

    try_set_editor_property(character_cdo, "add_default_movement_bindings", True)

    movement_component = try_get_editor_property(character_cdo, "movement_component")
    if movement_component:
        try_set_editor_property(movement_component, "constrain_to_plane", True)
        try_set_editor_property(movement_component, "plane_constraint_normal", unreal.Vector(0.0, 0.0, 1.0))
        try_set_editor_property(movement_component, "snap_to_plane_at_start", True)
        try_set_editor_property(movement_component, "max_speed", 850.0)
        try_set_editor_property(movement_component, "acceleration", 2400.0)
        try_set_editor_property(movement_component, "deceleration", 3200.0)
        try_set_editor_property(movement_component, "turning_boost", 8.0)

    mesh_component = try_get_editor_property(character_cdo, "mesh_component")
    if mesh_component:
        weapon_mesh = first_existing_asset([
            "/Game/Weapons/GrenadeLauncher/Meshes/SM_GrenadeLauncher.SM_GrenadeLauncher",
            "/Game/Weapons/GrenadeLauncher/Meshes/FirstPersonProjectileMesh.FirstPersonProjectileMesh",
            "/Game/Variant_Shooter/Blueprints/Pickups/Projectiles/Meshes/SM_FoamBullet.SM_FoamBullet",
            "/Engine/BasicShapes/Cube.Cube",
        ])

        if weapon_mesh:
            mesh_component.set_static_mesh(weapon_mesh)

        mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        try_set_editor_property(mesh_component, "relative_location", unreal.Vector(90.0, 32.0, -24.0))
        try_set_editor_property(mesh_component, "relative_rotation", unreal.Rotator(0.0, -90.0, 0.0))
        try_set_editor_property(mesh_component, "relative_scale3d", unreal.Vector(0.35, 0.35, 0.35))
        try_set_editor_property(mesh_component, "visible", True)
        try_set_editor_property(mesh_component, "hidden_in_game", False)

    save_asset_or_fail(ASSET_ROOT + "/Core/BP_RocketJumpCharacter")
    log("Configured playable pawn movement and weapon visual.")


def configure_game_mode():
    game_mode_class = load_blueprint_class(ASSET_ROOT + "/Core/BP_RocketJumpGameMode")
    character_class = (
        load_optional_blueprint_class("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter")
        or load_blueprint_class(ASSET_ROOT + "/Core/BP_RocketJumpCharacter")
    )
    controller_class = (
        load_optional_blueprint_class("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController")
        or load_blueprint_class(ASSET_ROOT + "/Core/BP_RocketJumpPlayerController")
    )
    hud_class = load_blueprint_class(ASSET_ROOT + "/Core/BP_RocketJumpHUD")

    game_mode_cdo = unreal.get_default_object(game_mode_class)
    game_mode_cdo.set_editor_property("default_pawn_class", character_class)
    game_mode_cdo.set_editor_property("player_controller_class", controller_class)
    game_mode_cdo.set_editor_property("hud_class", hud_class)

    save_asset_or_fail(ASSET_ROOT + "/Core/BP_RocketJumpGameMode")
    log("Configured GameMode defaults.")


def main():
    ensure_directory(ASSET_ROOT)

    for folder, asset_name, parent_class, description in BLUEPRINT_SPECS:
        package_path = ASSET_ROOT + "/" + folder
        ensure_directory(package_path)
        blueprint = recreate_blueprint_if_needed(asset_name, package_path, parent_class)
        unreal.EditorAssetLibrary.set_metadata_tag(blueprint, "RocketJumpDescription", description)
        save_asset_or_fail(package_path + "/" + asset_name)

    configure_game_mode()
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.EditorAssetLibrary.save_directory(ASSET_ROOT, only_if_is_dirty=False, recursive=True)

    missing_files = []
    for folder, asset_name, _, _ in BLUEPRINT_SPECS:
        asset_path = ASSET_ROOT + "/" + folder + "/" + asset_name
        filename = asset_path_to_filename(asset_path)
        if not os.path.exists(filename):
            missing_files.append(filename)

    if missing_files:
        raise RuntimeError("Blueprint files were not written:\n" + "\n".join(missing_files))

    log("Finished creating Rocket Jump Blueprint assets.")


if __name__ == "__main__":
    main()
