# Egypt — Pharaoh Forensic Gameplay Reference

Status: **Authoritative behavioural reference captured from the September 2026 design/review session**

Purpose: preserve the concrete gameplay behaviours observed and discussed during the live Pharaoh reference walkthrough so Egypt does not depend on chat history. This document records *behaviour and design lessons*, not Pharaoh assets, code, maps, UI, numbers or content to copy.

## 1. Presentation and camera

- Egypt is a **2.5D isometric city-builder** with a 2D logical simulation grid underneath.
- The visual target is a modernised Pharaoh-like angled city view: shallow orthographic/isometric presentation, readable buildings, small moving citizens, continuous-looking terrain and little visible grid clutter.
- The isometric handedness is locked to the reference orientation: logical +X runs down-left on screen and logical +Y runs down-right.
- Camera navigation is core gameplay, not debug functionality:
  - continuous edge scrolling;
  - right/middle mouse drag panning;
  - mouse-wheel zoom;
  - arrows/WASD panning;
  - Home/recentre;
  - configurable scroll speed;
  - display/window resolution options.

## 2. Visible causality rule

The central rule is stronger than merely showing buildings:

> **Important state changes should have a visible cause and a visible consequence in the city whenever practical.**

Examples established in the walkthrough:

- residents do not materialise in houses; immigrants enter the map and walk to homes;
- additional viable housing causes additional visible immigration traffic;
- workers travel from homes to workplaces;
- specialist workers visibly change role/appearance when they begin specialist work;
- hunters visibly leave the workplace, travel into the environment, hunt, return and contribute food;
- goods live in buildings/storage rather than in an unexplained global inventory;
- housing visibly improves as services and goods improve;
- neighbourhood growth changes both appearance and capacity.

## 3. Roads and immigration

Observed/reference behaviour:

1. The player places roads and housing.
2. Empty viable houses create housing demand.
3. Immigrants enter from a map/kingdom edge.
4. They visibly travel along the road network toward housing.
5. Population is added only when the immigrant group actually arrives.
6. Building additional viable houses creates additional immigration waves.

Egypt requirement:

- Road access alone must never create population.
- Food/service eligibility can create demand, but demand becomes population only after a visible immigrant agent reaches the house.
- A house not connected to a valid kingdom/map-edge road route cannot receive immigrants.
- In-transit immigrants reserve capacity so multiple groups do not overfill a house.

## 4. Food, storage and distribution

The reference walkthrough established the basic city food grammar:

**food source → worker/producer → physical collection/return → storage/granary → market/distribution → house consumption**

Current/near-term Egypt chain:

**farm and/or hunting lodge → granary → market → house**

Requirements:

- Food has physical stock in specific buildings.
- Granaries are real storage with finite capacity, not a cosmetic building.
- Markets hold inventory drawn from storage and supply nearby houses.
- Houses hold/consume household food stock.
- Eventually transport between these stages should be represented by visible workers/carts rather than instant transfer.
- The UI must expose stock levels and allow the player to trace shortages backwards.

## 5. Housing evolution

The walkthrough showed housing as a dynamic neighbourhood system, not fixed huts.

Established behaviour to preserve in spirit:

- housing starts modestly;
- services/goods improve housing automatically;
- the player does not click an "upgrade house" button;
- water is an early upgrade gate;
- improved housing becomes visually better;
- improved housing has greater resident capacity;
- additional capacity can trigger another visible immigration wave;
- compatible adjacent improved houses can visually/structurally merge into larger multi-tile residences;
- if important requirements disappear for long enough, housing can regress.

Egypt's exact level names, thresholds, sizes and capacities must be original.

### First Egypt housing ladder

This is a bootstrap ladder, not the final campaign progression:

- **Level 0 — Hut:** road + food eligibility, capacity 8.
- **Level 1 — Watered Home:** sustained food + well service, capacity 12.
- **Level 2 — Established Residence:** sustained food + well service + stable neighbourhood, capacity 16.

Later levels will add pottery, religion, beer, linen, healthcare, education, entertainment, desirability and luxury requirements.

### Multi-tile merging

Near-term implementation may keep separate logical household records while rendering compatible 2x2 upgraded clusters as one larger residence. Long-term simulation should support genuine multi-tile residence identity and capacity accounting.

## 6. Water service

- Wells are an early residential service.
- A well supplies houses within a bounded local radius; it is not a global city flag.
- Water coverage should be inspectable/overlayable.
- Sustained water + food allows housing to evolve and raises capacity.
- Removing water should eventually cause housing regression rather than an instantaneous visual flip.

## 7. Employment and visible workers

Reference behaviour observed:

1. Housing supplies residents/labour.
2. A workplace creates labour demand.
3. A resident visibly leaves home.
4. The resident travels by road to the workplace.
5. On arrival, the worker becomes the job-specific role.
6. The specialist then performs visible work.

Egypt requirement:

- Do not use Pharaoh's old recruiter-touch abstraction as the authoritative labour model.
- A worker must have a believable home/job relationship.
- Employed citizens remain part of the resident population.
- Jobs reserve/recruit available adult labour from reachable houses.
- Travel route and distance matter.
- Worker state should be visible and inspectable.

## 8. Hunting lodge reference slice

The hunting lodge is the first explicit visible-employment production slice.

Target state machine:

**HOME → COMMUTING TO LODGE → HUNTER ROLE/OUTFIT → HUNTING TRIP → RETURNING WITH MEAT → LODGE/STORAGE DELIVERY → HOME/REPEAT**

Near-term simplification is acceptable as long as the visible causal chain exists.

Requirements:

- Hunting lodge requires workers.
- Workers originate from populated reachable housing.
- The commute is visible.
- Hunter appearance/state changes at or after arriving at the lodge.
- Hunter leaves the road/workplace area to perform a hunting trip in suitable terrain.
- Returning hunter adds food/meat stock to the lodge.
- Lodge stock can then enter the storage/distribution chain.

## 9. Granary/storage behaviour

- Granaries have finite capacity.
- Food remains physically associated with the granary until moved onward.
- The player should be able to inspect quantity and later accepted-resource policy.
- Storage should become a traffic/logistics node: producers deliver in, buyers/carts carry out.
- A city with production but inadequate storage should visibly back up rather than silently discarding/teleporting goods.

## 10. Visual city readability

The screenshots reinforce these presentation rules:

- terrain should appear continuous rather than as obvious diamond graph paper;
- roads should read as continuous routes;
- citizens should be small but visible and numerous enough to show city activity;
- upgraded housing must look materially different, not just display a new number;
- multi-house neighbourhoods should visually knit together as they improve;
- worker roles should be distinguishable through silhouette/colour/animation;
- wildlife and birds help the city/world remain alive independently of economic actors.

## 11. Implementation priority captured from the walkthrough

Immediate order:

1. Preserve/fix 2.5D orientation and camera controls.
2. Visible immigrant agents and map-edge road entry.
3. Physical food stocks: farm → granary → market → house.
4. Wells and housing-level/capacity evolution.
5. Visual multi-house merging for upgraded 2x2 clusters.
6. Labour availability tied to residents.
7. Hunting lodge and visible worker/hunter state machine.
8. Granary/storage logistics and visible deliveries.
9. Replace remaining procedural/debug building art with production-quality original assets.
10. Add diagnostics/overlays so each link can be inspected.

## 12. Common library rule

Egypt uses **Infiltratr Common 1.16.0** for genuinely reusable infrastructure. Game-specific road topology, housing rules, resource chains and agent semantics remain in Egypt unless/until they become genuinely reusable across projects. Do not duplicate functionality already present in Common, but do not force Egypt-specific game logic into Common merely to claim reuse.

## 13. Guardrail

When implementing a new feature from the Pharaoh reference, ask:

- What caused this state change?
- Can the player see that cause?
- Is something physically located/moving when it ought to be?
- Can the player trace a failure?
- Does the system still make sense without the renderer?
- Are we preserving the useful gameplay idea while creating original code, art, balance and content?

If those answers are poor, the feature is not finished.

## 14. Road use and organic road evolution

The reference walkthrough showed that a road is not visually frozen forever after placement. Frequently used routes become more established-looking over time.

Egypt requirement:

- Road tiles accumulate **traffic from actual agents that traverse them**.
- Immigration groups contribute traffic proportional to the visible group using the route.
- Commuting workers contribute traffic on outbound and return journeys.
- Traffic history belongs to the simulation tile, not only to the renderer.
- Road appearance should evolve from a fresh track toward a visibly established route as cumulative use rises.
- The first bootstrap states are **New Track → Worn Road → Established Road**.
- These names/thresholds are original bootstrap values and can be rebalanced later.
- Future systems may make heavily used roads affect travel speed, maintenance, congestion or desirability, but road quality must not become an unexplained global upgrade.
- Inspecting a road should expose its traffic/use state.

## 15. House inspection and causal diagnostics

The reference house panel is a major design lesson, not incidental UI. Clicking a residence should answer **what is here, what it has, and exactly why it can or cannot improve**.

Egypt's house inspector should grow toward showing:

- current housing type/level;
- occupants and spare capacity;
- employed residents;
- food held by the household;
- other goods as those systems are introduced;
- water/service access;
- road access;
- taxation once taxation exists;
- health, fire, collapse and crime only after those systems genuinely exist;
- neighbourhood/desirability once implemented;
- a plain-language **next evolution blocker**.

Examples of valid current diagnostic messages include:

- `NEEDS ROAD ACCESS`
- `NEEDS A RELIABLE FOOD SUPPLY`
- `NEEDS WATER FROM A NEARBY WELL`
- `FOOD AND WATER ARE STABILISING`
- `SUSTAIN FOOD AND WATER TO EVOLVE`
- `NEEDS MORE GOODS AND SERVICES FOR NEXT LEVEL`

Do not fill the inspector with fake green statuses for systems that are not yet simulated. Add a field when the underlying simulation exists.

## 16. Map navigation, minimap, time and simulation controls

The walkthrough established several UI elements as functional requirements:

- a whole-map minimap/overview in the lower-left;
- the visible camera viewport shown on the minimap;
- minimap navigation should eventually allow rapid repositioning of the main camera;
- a flat/diagnostic building-footprint mode is useful and should remain available even after final art improves;
- simulation controls must include pause and multiple speed levels;
- the current month/year should be visible because time is part of the simulation, not merely decoration;
- population and treasury belong in the primary simulation HUD.

These systems should be original in presentation while preserving the useful information density of the reference.

## 17. Atmospheric world layer

The walkthrough also showed a lightweight atmospheric layer over the city:

- moving cloud shadows pass across terrain and buildings;
- water has visible surface motion;
- birds/wildlife move independently of economic actors;
- vegetation and ambient activity keep the map alive even when the player is not placing buildings.

Egypt should add atmospheric effects as presentation driven by deterministic/lightweight state where practical. Cloud shadows are visual atmosphere and must not be confused with terrain fertility or gameplay darkness unless a later weather system deliberately gives them simulation effects.

## 18. Current implementation checkpoint

As of this forensic pass:

- visible immigration, water-driven housing growth, visible employment/hunters, hunting food, granary stock and housing capacity are implemented in bootstrap form;
- roads now accumulate traffic from actual immigrant and worker movement and expose three use/evolution levels;
- the simulation exposes plain-language housing evolution diagnostics;
- the richer full-screen house panel, road visual art states, minimap, time/speed HUD, flat diagnostic view and moving cloud shadows remain presentation work to build on top of those simulation hooks.

The distinction matters: a behaviour is only marked implemented here when the underlying simulation state exists, not merely because the reference screenshot has been documented.
