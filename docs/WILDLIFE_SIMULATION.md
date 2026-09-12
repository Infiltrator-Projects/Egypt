# Egypt — Wildlife and Hunting Simulation

Status: implemented bootstrap system, 12 Sep 2026.

This note records the wildlife behaviour recovered from the Pharaoh reference walkthrough and implemented in Egypt. It supersedes older notes that describe hunting as targeting abstract terrain rather than a real animal.

## Core rule

Hunting obeys the project's visible-causality rule. A hunting lodge does not generate food merely because a timer expires.

The current chain is:

**resident home → road commute → hunting lodge → hunter role → select an actual wildlife agent → cross the landscape to that animal → hunt → animal is removed from the wildlife population → hunter visibly returns carrying meat → meat enters lodge stock → physical food logistics can move it onward**

If a hunter cannot find reachable wildlife, the worker performs no successful hunt and returns no meat. Food is therefore causally tied to an animal that existed in the world.

## Wildlife agents

The bootstrap world currently contains gazelle agents.

Each wildlife agent has:

- a persistent simulation id;
- a map position;
- a wildlife kind;
- deterministic lightweight wandering behaviour.

Wildlife avoids Nile water, reeds and occupied building tiles while wandering. Animals that have been selected by a hunter are reserved so a second hunter does not claim the same target and the target does not wander away while the first physical hunting interaction is being completed.

A small deterministic replenishment mechanism prevents the bootstrap map from being permanently emptied while the broader ecology system is still undeveloped. Population numbers, replenishment rates and species mix are balance/content values rather than final design.

## Hunter behaviour

A hunter arriving at a lodge searches for reachable unreserved wildlife rather than choosing an arbitrary four-tile excursion.

The hunter uses terrain pathfinding to the selected animal. This field path is deliberately separate from road commuting: residents use roads to reach employment, while a hunter can then leave the road network to cross suitable land.

When the hunt completes successfully:

1. the exact target wildlife id is harvested;
2. that animal disappears from the authoritative wildlife collection;
3. the world's wildlife-harvest counter advances;
4. the hunter receives a meat payload;
5. the hunter retraces the field route to the lodge;
6. lodge food stock increases only when the hunter returns.

The smoke test explicitly checks that a hunter selected a real wildlife id and that the authoritative wildlife harvest counter advanced. This prevents regression to an invisible `+food` hunting timer.

## Presentation

Gazelles are now drawn as small moving animals in the isometric world. They are simulation agents rather than decorative particles.

This is still programmer art. Final assets need readable animation, scale and species variation, but visual replacement must preserve the simulation identity underneath.

## Next ecology work

The current implementation is intentionally the smallest coherent ecology layer. Later passes should add:

- additional huntable species where historically/scenario appropriate;
- birds/ibis and other ambient animals, with a clear distinction between decorative and economically meaningful wildlife;
- better animal movement and avoidance;
- herd behaviour where useful;
- scenario/ecology-sensitive replenishment rather than the current bootstrap replacement rule;
- hunting pressure and local depletion that matters strategically;
- inspection/overlay support for wildlife and hunting availability.

The guardrail remains: if hunting changes city food supply, the player should be able to see and trace what was hunted and how the food got back into the city.
