#pragma once

#include "scene/animation/animation_tree.h"

#include <cassert>

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

struct AnimationData {
	struct TrackValue {
		Animation::Track *track = nullptr;
	};

	struct PositionTrackValue : public TrackValue {
		int bone_idx = -1;
		Vector3 position = Vector3(0, 0, 0);
	};

	struct RotationTrackValue : public TrackValue {
		int bone_idx = -1;
		Quaternion rotation = Quaternion(0, 0, 0, 1);
	};

	struct ScaleTrackValue : public TrackValue {
		int bone_idx = -1;
		Vector3 scale;
	};

	AHashMap<Animation::Track, TrackValue *, HashHasher, HashMapComparatorDefault<unsigned>> track_values;
};

struct GraphEvaluationContext {
	AnimationTree *animation_tree = nullptr;
};

class SyncedAnimationNode {
public:
	struct NodeTimeInfo {
		double length = 0.0;
		double position = 0.0;
		double delta = 0.0;

		Animation::LoopMode loop_mode = Animation::LOOP_NONE;
	};
	NodeTimeInfo node_time_info;

	virtual ~SyncedAnimationNode() = default;
	virtual void initialize(GraphEvaluationContext &context) {}
	virtual void activate_inputs(Vector<StringName> input_names) {}
	virtual void calculate_sync_track() {}
	virtual void update_time(double p_delta) {
		node_time_info.position += p_delta;
		if (node_time_info.position > node_time_info.length) {
			switch (node_time_info.loop_mode) {
				case Animation::LOOP_NONE: {
					node_time_info.position = node_time_info.length;
					break;
				}
				case Animation::LOOP_LINEAR: {
					assert(node_time_info.length > 0.0);
					while (node_time_info.position > node_time_info.length) {
						node_time_info.position -= node_time_info.length;
					}
					break;
				}
				case Animation::LOOP_PINGPONG: {
					assert(false && !"Not yet implemented.");
					break;
				}
			}
		}
	}
	virtual void evaluate(AnimationData &output) {}
};

class AnimationSamplerNode : public SyncedAnimationNode {
	StringName animation_name;
	Ref<Animation> animation;

	void initialize(GraphEvaluationContext &context) override;
	void evaluate(AnimationData &output) override;
};