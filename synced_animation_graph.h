#pragma once

#include "scene/animation/animation_blend_tree.h"
#include "scene/animation/animation_tree.h"

class SyncedAnimationGraph : public Node {
	GDCLASS(SyncedAnimationGraph, Node);

private:
	NodePath animation_tree_path;
	NodePath skeleton_path;

	void set_animation_tree(const NodePath &p_path);
	NodePath get_animation_tree() const;

	void set_skeleton(const NodePath &p_path);
	NodePath get_skeleton() const;

	// AnimationMixer::TrackCache
	
protected:
	void _notification(int p_what);
	static void _bind_methods();

	/* ---- General settings for animation ---- */
	AnimationMixer::AnimationCallbackModeProcess callback_mode_process = AnimationMixer::ANIMATION_CALLBACK_MODE_PROCESS_IDLE;
	AnimationMixer::AnimationCallbackModeMethod callback_mode_method = AnimationMixer::ANIMATION_CALLBACK_MODE_METHOD_DEFERRED;
	AnimationMixer::AnimationCallbackModeDiscrete callback_mode_discrete = AnimationMixer::ANIMATION_CALLBACK_MODE_DISCRETE_RECESSIVE;

	bool processing = false;
	bool active = true;

public:
	void _ready(const NodePath &p_path);
	void _process_graph(double p_delta, bool p_update_only = false);

	void set_active(bool p_active);
	bool is_active() const;

	void set_callback_mode_process(AnimationMixer::AnimationCallbackModeProcess p_mode);
	AnimationMixer::AnimationCallbackModeProcess get_callback_mode_process() const;

	void set_callback_mode_method(AnimationMixer::AnimationCallbackModeMethod p_mode);
	AnimationMixer::AnimationCallbackModeMethod get_callback_mode_method() const;

	void set_callback_mode_discrete(AnimationMixer::AnimationCallbackModeDiscrete p_mode);
	AnimationMixer::AnimationCallbackModeDiscrete get_callback_mode_discrete() const;

	SyncedAnimationGraph();

private:
	void _set_process(bool p_process, bool p_force = false);
};