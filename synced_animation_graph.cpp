#include "synced_animation_graph.h"

#include "core/os/time.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/animation/animation_player.h"

void SyncedAnimationGraph::_bind_methods() {
	print_line(vformat("binding methods"));

	ClassDB::bind_method(D_METHOD("set_active", "active"), &SyncedAnimationGraph::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &SyncedAnimationGraph::is_active);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");

	ClassDB::bind_method(D_METHOD("set_callback_mode_process", "mode"), &SyncedAnimationGraph::set_callback_mode_process);
	ClassDB::bind_method(D_METHOD("get_callback_mode_process"), &SyncedAnimationGraph::get_callback_mode_process);

	ClassDB::bind_method(D_METHOD("set_callback_mode_method", "mode"), &SyncedAnimationGraph::set_callback_mode_method);
	ClassDB::bind_method(D_METHOD("get_callback_mode_method"), &SyncedAnimationGraph::get_callback_mode_method);

	ClassDB::bind_method(D_METHOD("set_animation_tree", "animation_tree"), &SyncedAnimationGraph::set_animation_tree);
	ClassDB::bind_method(D_METHOD("get_animation_tree"), &SyncedAnimationGraph::get_animation_tree);
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "animation_tree", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "AnimationTree"), "set_animation_tree", "get_animation_tree");

	ClassDB::bind_method(D_METHOD("set_skeleton", "skeleton"), &SyncedAnimationGraph::set_skeleton);
	ClassDB::bind_method(D_METHOD("get_skeleton"), &SyncedAnimationGraph::get_skeleton);
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "skeleton", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Skeleton3D"), "set_skeleton", "get_skeleton");

	ADD_SIGNAL(MethodInfo(SNAME("animation_tree_changed")));
}

void SyncedAnimationGraph::_notification(int p_what) {
	switch (p_what) {
		case Node::NOTIFICATION_READY: {
			_set_process(true);
		} break;

		case Node::NOTIFICATION_INTERNAL_PROCESS: {
			if (active) {
				_process_graph(get_process_delta_time());
			}
		} break;

		case Node::NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			if (active) {
				_process_graph(get_physics_process_delta_time());
			}
		} break;
	}
}

void SyncedAnimationGraph::set_active(bool p_active) {
	if (active == p_active) {
		return;
	}

	active = p_active;
	_set_process(processing, true);
}

bool SyncedAnimationGraph::is_active() const {
	return active;
}

void SyncedAnimationGraph::set_callback_mode_process(AnimationMixer::AnimationCallbackModeProcess p_mode) {
	if (callback_mode_process == p_mode) {
		return;
	}

	bool was_active = is_active();
	if (was_active) {
		set_active(false);
	}

	callback_mode_process = p_mode;

	if (was_active) {
		set_active(true);
	}
}

AnimationMixer::AnimationCallbackModeProcess SyncedAnimationGraph::get_callback_mode_process() const {
	return callback_mode_process;
}

void SyncedAnimationGraph::set_callback_mode_method(AnimationMixer::AnimationCallbackModeMethod p_mode) {
	callback_mode_method = p_mode;
	emit_signal(SNAME("mixer_updated"));
}

AnimationMixer::AnimationCallbackModeMethod SyncedAnimationGraph::get_callback_mode_method() const {
	return callback_mode_method;
}

void SyncedAnimationGraph::set_callback_mode_discrete(AnimationMixer::AnimationCallbackModeDiscrete p_mode) {
	callback_mode_discrete = p_mode;
	emit_signal(SNAME("mixer_updated"));
}

AnimationMixer::AnimationCallbackModeDiscrete SyncedAnimationGraph::get_callback_mode_discrete() const {
	return callback_mode_discrete;
}

void SyncedAnimationGraph::set_animation_tree(const NodePath &p_path) {
	animation_tree_path = p_path;
	if (p_path.is_empty()) {
		//		set_root_node(SceneStringName(path_pp));
		//		while (animation_libraries.size()) {
		//			remove_animation_library(animation_libraries[0].name);
		//		}
	}
	emit_signal(SNAME("animation_tree_changed")); // Needs to unpin AnimationPlayerEditor.
}

NodePath SyncedAnimationGraph::get_animation_tree() const {
	return animation_tree_path;
}

void SyncedAnimationGraph::set_skeleton(const NodePath &p_path) {
	skeleton_path = p_path;
	if (p_path.is_empty()) {
		//		set_root_node(SceneStringName(path_pp));
		//		while (animation_libraries.size()) {
		//			remove_animation_library(animation_libraries[0].name);
		//		}
	}
	emit_signal(SNAME("skeleton_changed")); // Needs to unpin AnimationPlayerEditor.
}

NodePath SyncedAnimationGraph::get_skeleton() const {
	return skeleton_path;
}

void SyncedAnimationGraph::_ready(const NodePath &p_path) {
	print_line(vformat("synced animation graph ready!"));
}

void SyncedAnimationGraph::_process_graph(double p_delta, bool p_update_only) {
	if (skeleton_path.is_empty()) {
		return;
	}

	float current_time = Time::get_singleton()->get_unix_time_from_system();
	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(get_node_or_null(skeleton_path));
	if (!skeleton) {
		return;
	}

	AnimationTree *animation_tree = Object::cast_to<AnimationTree>(get_node_or_null(animation_tree_path));
	if (!animation_tree) {
		return;
	}

	Ref<Animation> animation = animation_tree->get_animation("Walk-InPlace");
	if (!animation.is_valid()) {
		return;
	}

	static double debug_time = 0.;
	debug_time += p_delta;
	if (debug_time > 2.0) {
		debug_time = 0.;
	}

	current_time = debug_time;

	//	LocalVector<AnimationMixer::TrackCache *> &track_num_to_track_cache = animation_track_num_to_track_cache[a];
	const Vector<Animation::Track *> tracks = animation->get_tracks();
	Animation::Track *const *tracks_ptr = tracks.ptr();
	// real_t a_length = animation->get_length();
	int count = tracks.size();
	for (int i = 0; i < count; i++) {
		const Animation::Track *animation_track = tracks_ptr[i];
		if (!animation_track->enabled) {
			continue;
		}

		Animation::TrackType ttype = animation_track->type;
		switch (ttype) {
			case Animation::TYPE_POSITION_3D: {
				AnimationMixer::TrackCacheTransform *track_xform = memnew(AnimationMixer::TrackCacheTransform);
				track_xform->type = Animation::TYPE_POSITION_3D;
				track_xform->bone_idx = -1;
				track_xform->skeleton_id = skeleton->get_instance_id();
				NodePath path = animation->track_get_path(i);

				double animation_time = Math::fposmod(current_time, animation->get_length());
				if (path.get_subname_count() == 1) {
					int bone_idx = skeleton->find_bone(path.get_subname(0));
					if (bone_idx != -1) {
						track_xform->bone_idx = bone_idx;
						Vector3 pos;
						animation->try_position_track_interpolate(i, animation_time, &pos);
						skeleton->set_bone_pose_position(bone_idx, pos);
					}
				}
				break;
			}
			case Animation::TYPE_ROTATION_3D: {
				AnimationMixer::TrackCacheTransform *track_xform = memnew(AnimationMixer::TrackCacheTransform);
				track_xform->type = Animation::TYPE_POSITION_3D;
				track_xform->bone_idx = -1;
				track_xform->skeleton_id = skeleton->get_instance_id();
				NodePath path = animation->track_get_path(i);

				double animation_time = Math::fposmod(current_time, animation->get_length());
				if (path.get_subname_count() == 1) {
					int bone_idx = skeleton->find_bone(path.get_subname(0));
					if (bone_idx != -1) {
						track_xform->bone_idx = bone_idx;
						Quaternion rot;
						animation->try_rotation_track_interpolate(i, animation_time, &rot);
						skeleton->set_bone_pose_rotation(bone_idx, rot);
					}
				}
				break;
			}
			default: {
				break;
			}
		}
	}

	//	skeleton->set_bone_pose_position(3, Vector3(sin(current_time) * 10., 0., 0.));
	skeleton->force_update_all_bone_transforms();
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

void AnimationSamplerNode::initialize(GraphEvaluationContext &context) {
	animation = context.animation_tree->get_animation(animation_name);
}


void AnimationSamplerNode::evaluate(GraphEvaluationContext &context, AnimationData &output) {
	const Vector<Animation::Track *> tracks = animation->get_tracks();
	Animation::Track *const *tracks_ptr = tracks.ptr();
	// real_t a_length = animation->get_length();
	int count = tracks.size();
	for (int i = 0; i < count; i++) {
		AnimationData::TrackValue *track_value = nullptr;
		const Animation::Track *animation_track = tracks_ptr[i];
		const NodePath& track_node_path = animation_track->path;
		if (!animation_track->enabled) {
			continue;
		}

		Animation::TrackType ttype = animation_track->type;
		switch (ttype) {
			case Animation::TYPE_POSITION_3D: {
				AnimationData::PositionTrackValue *position_track_value = memnew(AnimationData::PositionTrackValue);

				if (track_node_path.get_subname_count() == 1) {
					int bone_idx = context.skeleton_3d->find_bone(track_node_path.get_subname(0));
					if (bone_idx != -1) {
						position_track_value->bone_idx = bone_idx;
						animation->try_position_track_interpolate(i, node_time_info.position, &position_track_value->position);
					}
				} else {
					// TODO
					assert(false && !"Not yet implemented");
				}

				track_value = position_track_value;
				break;
			}
			case Animation::TYPE_ROTATION_3D: {
				AnimationData::RotationTrackValue *rotation_track_value = memnew(AnimationData::RotationTrackValue);

				if (track_node_path.get_subname_count() == 1) {
					int bone_idx = context.skeleton_3d->find_bone(track_node_path.get_subname(0));
					if (bone_idx != -1) {
						rotation_track_value->bone_idx = bone_idx;
						animation->try_rotation_track_interpolate(i, node_time_info.position, &rotation_track_value->rotation);
					}
				} else {
					// TODO
					assert(false && !"Not yet implemented");
				}

				track_value = rotation_track_value;
				break;
			}
			default: {
				// TODO
				assert(false && !"Not yet implemented");
				break;
			}
		}

		output.set_value(animation_track->thash, track_value);
	}
}