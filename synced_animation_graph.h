#pragma once

#include "scene/animation/animation_blend_tree.h"
#include "scene/animation/animation_tree.h"

class SyncedAnimationGraph : public AnimationMixer {
	GDCLASS(SyncedAnimationGraph, AnimationMixer);

private:
	bool started = true;

	friend class AnimationNode;

	Ref<AnimationRootNode> root_animation_node;

	mutable bool properties_dirty = true;

	void _update_properties() const;

	NodePath animation_player;
	void set_animation_player(const NodePath &p_path);
	NodePath get_animation_player() const;

	void set_root_animation_node(const Ref<AnimationRootNode> &p_animation_node);
	Ref<AnimationRootNode> get_root_animation_node() const;

	void _tree_changed();

	void _setup_animation_player();
	void _animation_player_changed();

	void _notification(int p_what);

	virtual void _set_active(bool p_active) override;

protected:
	static void _bind_methods();

public:
	void _process_animation(double p_delta, bool p_update_only = false) override;

	SyncedAnimationGraph();
};