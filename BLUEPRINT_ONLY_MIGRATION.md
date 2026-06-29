# Blueprint-Only Branch

This branch removes the native `RocketJump` C++ module from the Unreal project.

Open `rocket_jump_blueprint.uproject` or run `OpenRocketJumpProject.bat` to start the Blueprint-only project in Unreal Engine 5.7.

Run `CreateRocketJumpBlueprints.bat` after Unreal Engine is installed to generate the Blueprint assets under `/Game/Blueprints`.

Notes:

- `rocket_jump.uproject` and `rocket_jump_blueprint.uproject` do not declare any C++ modules.
- `Config/DefaultEngine.ini` no longer points to `/Script/RocketJump.RocketJumpGameMode`.
- The old `Source` module, editor targets, rebuild batch file, Visual Studio config, and upgrade log were removed from this branch.
- `Content/Editor/CreateRocketJumpBlueprints.py` creates the initial Blueprint class assets and wires `BP_RocketJumpGameMode` to the generated character, controller, and HUD classes.

Generated Blueprint classes:

- `/Game/Blueprints/Core/BP_RocketJumpGameMode`
- `/Game/Blueprints/Core/BP_RocketJumpCharacter`
- `/Game/Blueprints/Core/BP_RocketJumpPlayerController`
- `/Game/Blueprints/Core/BP_RocketJumpHUD`
- `/Game/Blueprints/Components/BP_RocketJumpComponent`
- `/Game/Blueprints/Components/BP_RocketJumpHealthComponent`
- `/Game/Blueprints/Components/BP_RotatingPlatformComponent`
- `/Game/Blueprints/Gameplay/BP_RocketJumpProjectile`
- `/Game/Blueprints/Gameplay/BP_RocketJumpTurret`
- `/Game/Blueprints/Gameplay/BP_TurretProjectile`
- `/Game/Blueprints/Gameplay/BP_RocketJumpPickup`
- `/Game/Blueprints/Gameplay/BP_RocketJumpLaserBeam`
- `/Game/Blueprints/World/BP_LevelSetup`
- `/Game/Blueprints/World/BP_ShowcaseSetup`
