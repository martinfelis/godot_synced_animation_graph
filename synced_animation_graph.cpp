#include "synced_animation_graph.h"

#include "scene/animation/animation_player.h"

void SyncedAnimationGraph::_bind_methods() {
	print_line(vformat("binding methods"));

	ClassDB::bind_method(D_METHOD("set_tree_root", "animation_node"), &SyncedAnimationGraph::set_root_animation_node);
	ClassDB::bind_method(D_METHOD("get_tree_root"), &SyncedAnimationGraph::get_root_animation_node);

	ClassDB::bind_method(D_METHOD("set_animation_player", "path"), &SyncedAnimationGraph::set_animation_player);
	ClassDB::bind_method(D_METHOD("get_animation_player"), &SyncedAnimationGraph::get_animation_player);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tree_root", PROPERTY_HINT_RESOURCE_TYPE, "AnimationRootNode"), "set_tree_root", "get_tree_root");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "anim_player", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "AnimationPlayer"), "set_animation_player", "get_animation_player");

	ADD_SIGNAL(MethodInfo(SNAME("animation_player_changed")));
}

void SyncedAnimationGraph::_set_active(bool p_active) {
	_set_process(p_active);
	started = p_active;
}

void SyncedAnimationGraph::_process_animation(double p_delta, bool p_update_only) {
	print_line(vformat("updating blend tree! %f", p_delta));
	if (!root_animation_node.is_valid()) {
		return;
	}

	Ref<AnimationNodeBlendTree> blend_tree = root_animation_node;
	if (!blend_tree.is_valid()) {
		print_line("Cannot process animation graph: root not AnimationNodeBlendTree");
		return;
	}

	LocalVector<StringName> node_names = blend_tree->get_node_list();
	for (StringName node_name : node_names) {
		print_line(vformat("  %s", node_name));
	}
}

SyncedAnimationGraph::SyncedAnimationGraph() {
}

void SyncedAnimationGraph::_update_properties() const {
	if (!properties_dirty) {
		return;
	}

	properties_dirty = false;

	const_cast<SyncedAnimationGraph *>(this)->notify_property_list_changed();
}

void SyncedAnimationGraph::set_root_animation_node(const Ref<AnimationRootNode> &p_animation_node) {
	if (root_animation_node.is_valid()) {
		root_animation_node->disconnect(SNAME("tree_changed"), callable_mp(this, &SyncedAnimationGraph::_tree_changed));
		//		root_animation_node->disconnect(SNAME("animation_node_renamed"), callable_mp(this, &SyncedAnimationGraph::_animation_node_renamed));
		//		root_animation_node->disconnect(SNAME("animation_node_removed"), callable_mp(this, &SyncedAnimationGraph::_animation_node_removed));
	}

	root_animation_node = p_animation_node;

	if (root_animation_node.is_valid()) {
		root_animation_node->connect(SNAME("tree_changed"), callable_mp(this, &SyncedAnimationGraph::_tree_changed));
		//		root_animation_node->connect(SNAME("animation_node_renamed"), callable_mp(this, &SyncedAnimationGraph::_animation_node_renamed));
		//		root_animation_node->connect(SNAME("animation_node_removed"), callable_mp(this, &SyncedAnimationGraph::_animation_node_removed));
	}

	properties_dirty = true;

	update_configuration_warnings();
}

Ref<AnimationRootNode> SyncedAnimationGraph::get_root_animation_node() const {
	return root_animation_node;
}

void SyncedAnimationGraph::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_setup_animation_player();
			if (active) {
				_set_process(true);
			}
		} break;
	}
}

void SyncedAnimationGraph::_tree_changed() {
	if (properties_dirty) {
		return;
	}

	callable_mp(this, &SyncedAnimationGraph::_update_properties).call_deferred();
	properties_dirty = true;
}

void SyncedAnimationGraph::set_animation_player(const NodePath &p_path) {
	animation_player = p_path;
	if (p_path.is_empty()) {
		set_root_node(SceneStringName(path_pp));
		while (animation_libraries.size()) {
			remove_animation_library(animation_libraries[0].name);
		}
	}
	emit_signal(SNAME("animation_player_changed")); // Needs to unpin AnimationPlayerEditor.
	_setup_animation_player();
	notify_property_list_changed();
}

NodePath SyncedAnimationGraph::get_animation_player() const {
	return animation_player;
}

void SyncedAnimationGraph::_setup_animation_player() {
	if (!is_inside_tree()) {
		return;
	}

	cache_valid = false;

	if (animation_player.is_empty()) {
		clear_caches();
		return;
	}

	// Using AnimationPlayer here is for compatibility. Changing to AnimationMixer needs extra work like error handling.
	AnimationPlayer *player = Object::cast_to<AnimationPlayer>(get_node_or_null(animation_player));
	if (player) {
		if (!player->is_connected(SNAME("caches_cleared"), callable_mp(this, &SyncedAnimationGraph::_setup_animation_player))) {
			player->connect(SNAME("caches_cleared"), callable_mp(this, &SyncedAnimationGraph::_setup_animation_player), CONNECT_DEFERRED);
		}
		if (!player->is_connected(SNAME("animation_list_changed"), callable_mp(this, &SyncedAnimationGraph::_setup_animation_player))) {
			player->connect(SNAME("animation_list_changed"), callable_mp(this, &SyncedAnimationGraph::_setup_animation_player), CONNECT_DEFERRED);
		}
		Node *root = player->get_node_or_null(player->get_root_node());
		if (root) {
			set_root_node(get_path_to(root, true));
		}
		while (animation_libraries.size()) {
			remove_animation_library(animation_libraries[0].name);
		}
		List<StringName> list;
		player->get_animation_library_list(&list);
		for (const StringName &E : list) {
			Ref<AnimationLibrary> lib = player->get_animation_library(E);
			if (lib.is_valid()) {
				add_animation_library(E, lib);
			}
		}
	}

	clear_caches();
}
