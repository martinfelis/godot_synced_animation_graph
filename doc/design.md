## Graph

### 1. Generalized data connections / Support of math nodes (or non-AnimNodes in general)

Use case:

* Enables animators to add custom math for blend inputs or to adjust other inputs (e.g. LookAt or IK
  targets).

Effects on the graph topology?

* Need to generalize Output Sockets to different types instead of only "Animation Data".
* How to evaluate? Two types of subgraphs:
   a) Instant inputs (needed for blend node inputs) that have to be evaluated before
   UpdateConnections. Maybe restrict to data that is not animation data dependent?
   b) Processing nodes, e.g. for extracted bones.

### 2. Support of multiple output sockets

Use case:

* E.g. extract Bone transform and use in pose modifying nodes.
* Chain IK.

Effects on graph topology?

* Increases Node complexity:
    * AnimOutput
    * AnimOutput + Data
    * Data

(Data = bool, float, vec3, quat or ...)

**Open Issues**

1. Unclear when this is actually needed. Using more specific nodes that perform the desired logic
   may be better (
   c.f. https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-bone-driven-controller-in-unreal-engine).
   Likely this is not crucial so should be avoided for now.

### 3. Multi-skeleton evaluation

Use case: riding on a horse, interaction between two characters.

### 4. Output re-use

Description:

Output of a single node may be used as input of two or more other nodes.

Use case:

* (Related to 1.) an input that does some computation gets reused in two separate subtrees.

### 5. Inputs into Subgraphs

Description:

An embedded blend tree can receive inputs of the surrounding blend tree. Inputs are animations or - depending on 1. - also general input values.

Use case:

* Reuse of some blend logic or to move logic of a part of a blend tree into its own node.

Effects on graph topology:

* Great flexibility and possibly reusability.
* Improves logical block building.
* Probably only of bigger use with 1.

Open issues:

* Inputs to embedded state machines?
