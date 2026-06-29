import os
import unreal

OUT = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_saved_dir()), "InspectWorldApi.txt")

with open(OUT, "w", encoding="utf-8") as handle:
    for name in ["EditorLevelLibrary", "EditorActorSubsystem", "GameplayStatics", "PlayerController", "World"]:
        obj = getattr(unreal, name, None)
        handle.write(name + "=" + str(bool(obj)) + "\n")
        if obj:
            handle.write(name + ".dir=" + str(dir(obj)[:220]) + "\n")

unreal.log("[InspectWorldApi] OUT=" + OUT)
