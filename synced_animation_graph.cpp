#include "synced_animation_graph.h"

#include "scene/animation/animation_player.h"

void SyncedAnimationGraph::_bind_methods() {
	print_line(vformat("binding methods"));

	ClassDB::bind_method(D_METHOD("set_animation_tree", "animation_tree"), &SyncedAnimationGraph::set_animation_tree);
	ClassDB::bind_method(D_METHOD("get_animation_tree"), &SyncedAnimationGraph::get_animation_tree);
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "animation_tree", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "AnimationTree"), "set_animation_tree", "get_animation_tree");

	ADD_SIGNAL(MethodInfo(SNAME("animation_tree_changed")));
}

void SyncedAnimationGraph::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_set_process(true);
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			_process_animation(get_process_delta_time());
		} break;

		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			_process_animation(get_physics_process_delta_time());
		} break;
	}
}

void SyncedAnimationGraph::set_animation_tree(const NodePath &p_path) {
	animation_tree = p_path;
	if (p_path.is_empty()) {
		//		set_root_node(SceneStringName(path_pp));
		//		while (animation_libraries.size()) {
		//			remove_animation_library(animation_libraries[0].name);
		//		}
	}
	emit_signal(SNAME("animation_tree_changed")); // Needs to unpin AnimationPlayerEditor.
}

NodePath SyncedAnimationGraph::get_animation_tree() const {
	return animation_tree;
}

void SyncedAnimationGraph::_ready(const NodePath &p_path) {
	print_line(vformat("synced animation graph ready!"));
}

void SyncedAnimationGraph::_process_animation(double p_delta, bool p_update_only) {
	print_line(vformat("updating blend tree! %f", p_delta));
	//	if (!root_animation_node.is_valid()) {
	//		return;
	//	}
	//
	//	Ref<AnimationNodeBlendTree> blend_tree = root_animation_node;
	//	if (!blend_tree.is_valid()) {
	//		print_line("Cannot process animation graph: root not AnimationNodeBlendTree");
	//		return;
	//	}
	//
	//	LocalVector<StringName> node_names = blend_tree->get_node_list();
	//	for (StringName node_name : node_names) {
	//		print_line(vformat("  %s", node_name));
	//	}
}

void SyncedAnimationGraph::_set_process(bool p_process, bool p_force) {
	if (processing == p_process && !p_force) {
		return;
	}

	set_physics_process_internal(false);
	set_process_internal(true);

	processing = p_process;
}

SyncedAnimationGraph::SyncedAnimationGraph() {
}
