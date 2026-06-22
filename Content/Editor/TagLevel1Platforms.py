# level1'deki TUM StaticMesh actor'larina dondurme etiketi ekler (opsiyonel).
# Asil sistem artik Platform adi aramadan StaticMesh'leri dondurur; bu script sadece NoRotate dislamak icin.

import unreal

TAG_SKIP = "NoRotate"
editor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
count = 0

for actor in editor_subsystem.get_all_level_actors():
    comps = actor.get_components_by_class(unreal.StaticMeshComponent)
    if not comps:
        continue
    mesh = comps[0].static_mesh
    if not mesh:
        continue
    label = actor.get_actor_label().lower()
    if "floor" in label or "ground" in label or "zemin" in label:
        tags = list(actor.tags)
        if TAG_SKIP not in tags:
            tags.append(TAG_SKIP)
            actor.tags = tags
            count += 1

unreal.log(f"RocketJump: {count} zemin actor'una '{TAG_SKIP}' etiketi eklendi (donmez).")
