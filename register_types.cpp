#include "register_types.h"

#include "core/object/class_db.h"
#include "synced_animation_graph.h"

void initialize_synced_blend_tree_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<SyncedAnimationGraph>();
}

void uninitialize_synced_blend_tree_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	// Nothing to do here in this example.
}