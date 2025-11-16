#pragma once

#include "scene/animation/animation_blend_tree.h"
#include "scene/animation/animation_tree.h"

class SyncedAnimationGraph : public Node {
	GDCLASS(SyncedAnimationGraph, Node);

private:
	NodePath animation_tree;
	bool processing = false;

	void set_animation_tree(const NodePath &p_path);
	NodePath get_animation_tree() const;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void _ready(const NodePath &p_path);
	void _process_animation(double p_delta, bool p_update_only = false);

	SyncedAnimationGraph();

private:
	void _set_process(bool p_process, bool p_force = false);
};