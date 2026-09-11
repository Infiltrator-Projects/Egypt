# Pharaoh / Cleopatra Reference Bible

Status: **Reference research — not an implementation specification**

This document preserves the design research behind **Egypt** so the project does not depend on chat history. It records what made *Pharaoh* (1999) and *Cleopatra: Queen of the Nile* work, where their mechanics were limited by their era, and which ideas are useful as inspiration.

The purpose is **not** to reproduce copyrighted content, assets, scenarios, maps, dialogue, art, numerical balance or source code. Egypt is to be an original game. Where exact original mechanics are recorded below, they are here so we understand the reference design and can deliberately improve or depart from it.

---

## 1. Why Pharaoh is the primary reference

Pharaoh's strongest achievement was not any individual building. It was the way a large number of simple systems reinforced one another:

- housing evolved because residents gained access to progressively richer goods and services;
- raw materials were physically turned into manufactured goods;
- granaries, storage yards and bazaars made distribution spatial;
- walkers made services and logistics visible;
- roads affected almost every system;
- the Nile and annual inundation defined agriculture;
- work camps linked farming labour to monument labour;
- trade made local geography matter;
- gods affected civic management;
- health, education, entertainment and administration all fed housing progression and city ratings;
- monuments consumed years of material production and organised labour;
- missions gradually introduced systems across a historical campaign.

The enduring lesson for Egypt is:

> **The city itself is the progression system.**

The player becomes more capable by making the settlement itself more capable.

---

## 2. What the original designers considered central

Chris Beatrice's designer notes in the original manual make two ideas especially important.

First, the game is fundamentally about organising a large population to accomplish great civic works. Monument construction is deliberately multi-stage: materials are produced or imported, moved to the site, hauled by peasant labour, and worked by specialist guilds. The monument rises visibly rather than appearing after an abstract payment.

Second, the farming model is deliberately Egyptian rather than generic. The annual Nile inundation controls floodplain fertility and seasonal labour. When fields are underwater, peasants become available for monument work. Nilometers, granaries and irrigation all exist because the Nile cycle binds the society together.

Those two relationships — **population organisation → monument building** and **Nile cycle → agriculture → seasonal labour** — should remain fundamental reference points for Egypt.

---

## 3. Original high-level gameplay loop

A simplified reference loop is:

**land → roads → housing → immigration → food/water → workforce → industry → storage/distribution → services → housing evolution → taxation/trade → culture/religion/health → monuments/military → mission goals**

The important characteristic is that expansion creates new dependencies. Better houses are economically valuable, but they also require more complicated supply chains and services. Elite housing can even remove residents from the workforce by turning them into scribes, creating a deliberate tension between wealth and labour availability.

---

## 4. Housing evolution — reference behaviour

Housing begins as vacant residential lots. Immigrants build homes themselves. Housing improves when cumulative requirements are met and can devolve when required goods or services disappear.

The original progression is useful as a reference because it shows how the game gradually teaches its systems. The exact names, thresholds, populations and ordering are **not** requirements for Egypt.

Reference progression:

1. Vacant Lot — waits for immigrants.
2. Crude Hut — requires basic well water.
3. Sturdy Hut — requires food.
4. Meager Shanty — requires religious access.
5. Common Shanty — requires improved water delivery.
6. Rough Cottage — introduces entertainment.
7. Ordinary Cottage — introduces pottery.
8. Modest Homestead — introduces more advanced health service.
9. Spacious Homestead — introduces beer and broader entertainment.
10. Modest Apartment — introduces courthouse/administrative access.
11. Spacious Apartment — introduces education.
12. Common Residence — introduces food variety and dentistry.
13. Spacious Residence — introduces linen.
14. Elegant Residence — broadens religion and entertainment.
15. Fancy Residence — introduces luxury goods such as jewellery.
16. Manor classes — demand increasingly complete health, education, religion and entertainment.
17. Estate classes — demand broad food diversity and luxury provision.
18. Palatial Estate — top-end housing.

Important reference principles:

- requirements are cumulative;
- housing visually transforms as it evolves;
- higher housing generally produces more tax revenue and prosperity;
- neighbourhood desirability can block evolution even when material needs are met;
- better housing can improve nearby desirability;
- poor housing can worsen nearby desirability;
- elite/scribe-class housing changes the labour economy, so housing evolution is not a pure upgrade with no trade-off.

### Lesson for Egypt

Keep **emergent housing progression**, but build an original ladder with original thresholds and more legible household/neighbourhood state. Do not simply copy Pharaoh's exact requirement order.

---

## 5. Desirability and neighbourhood form

Pharaoh uses desirability as a spatial quality measure. Industry, military or unpleasant civic structures can lower it. Gardens, statues, plazas and impressive civic/religious structures can raise it.

This matters because housing progression is not solely a supply-chain challenge. Players must also create good neighbourhoods.

The problem is that desirability can become a somewhat opaque numerical field, and optimal residential blocks tend to converge toward repeated templates.

### Lesson for Egypt

Retain neighbourhood quality, but expose *why* an area is attractive or unattractive. Potential contributors can include:

- noise;
- smoke/dust;
- traffic;
- sewage/cleanliness;
- water access;
- gardens/shade;
- prestige buildings;
- crime;
- crowding;
- proximity to industrial freight;
- river view/access;
- religious/cultural importance.

This is reference direction only; exact systems belong in GAME_DESIGN.md.

---

## 6. Nile, inundation and farming

The Nile is one of Pharaoh's defining mechanics.

Reference behaviour:

- floodplain farms operate differently from meadow farms;
- floodplain land has fertility;
- inundation replenishes fertility;
- poor or absent flooding can reduce agricultural output;
- floodplain farms cannot be worked while inundated;
- a Nilometer provides information about the coming flood;
- irrigation ditches and water lifts can improve farm fertility;
- irrigation improves viable farmland but does not magically turn arbitrary desert into farmland;
- some regions support only certain crops;
- floodplain farms use workers supplied from Work Camps rather than directly employing year-round farm labour;
- the same Work Camps supply peasant gangs to monuments when agriculture does not require them.

Reference crop/food set includes:

- grain;
- chickpeas;
- lettuce;
- pomegranates;
- figs;
- barley;
- flax;
- cattle/meat;
- fish;
- hunted game.

Grain is particularly interesting because it also produces straw, which feeds other production chains.

### Core lesson

The best part of the system is not simply 'farms flood'. It is that **climate, fertility, food security, labour and monument construction are connected**.

---

## 7. Work Camps and seasonal labour

This is a particularly important reference mechanic.

Work Camps generate peasant labour gangs. Their priority is normally floodplain farming during the agricultural season. When farms do not need them — especially during inundation — those workers can instead haul materials and support monument construction.

Distance matters because time spent walking to a farm or monument is time not spent working.

### Lesson for Egypt

Seasonal labour should remain one of our foundational design ideas, but it can be made more explicit and believable. The player should be able to see and understand where available labour is going and why.

---

## 8. Production and industry chains

Pharaoh's production system is memorable because a relatively small number of resources cross-connect into housing, services, trade, military and monuments.

### Reference chains

**Clay → Potter → Pottery**

Pottery is a household good and is also relevant to later specialist production in Cleopatra.

**Clay + Straw → Brickworks → Bricks**

Bricks feed monument construction.

**Reeds → Papyrus Maker → Papyrus**

Papyrus is consumed by Scribal Schools and Libraries and can be exported.

**Barley → Brewery → Beer**

Beer is a household good, supports high-end entertainment such as Senet Houses, and can be exported.

**Flax → Weaver → Linen**

Linen is a household good and is also required by Mortuaries.

**Grain → food + Straw by-product**

Straw can feed cattle and brickmaking.

**Gemstones → Jeweller → Jewellery**

Luxury household good and possible trade commodity.

**Copper → Weaponsmith → Weapons**

Military supply chain.

**Copper + Wood → Chariot Maker → Chariots**

Supports advanced military units.

**Gold Mine → Treasury**

Gold production contributes directly to city wealth rather than passing through a household market chain.

**Limestone / Plain Stone / Granite / Sandstone → storage/hauling → monuments**

Different monument types require different stone materials.

### Cleopatra-specific examples

The expansion adds specialist funerary/monument chains including:

- henna → paint;
- pottery + imported oil → lamps;
- artisans using materials to finish royal burial tombs;
- additional grave/funerary provisions.

The lamp chain is notable because it behaves like a tertiary industry: a manufactured good becomes an input to another manufactured good.

### Lesson for Egypt

Production should be understandable but richly interconnected. Shared inputs and useful by-products create stronger strategic choices than isolated one-resource/one-output recipes.

---

## 9. Storage, granaries and distribution

### Granaries

Granaries primarily handle food. Their location strongly affects how efficiently markets can supply homes.

### Storage yards

Storage Yards hold raw materials, manufactured goods, imports and exports. Player storage policies can determine which goods are accepted, refused, stockpiled or made available.

### Bazaars

Bazaars are a central part of Pharaoh's visible economy:

1. buyers leave the bazaar;
2. they obtain food/goods from granaries or storage;
3. they return to the bazaar;
4. bazaar sellers walk through residential streets;
5. houses receive goods when sellers pass.

This means a shortage can be caused by production, storage location, buyer travel time, distribution pathing or insufficient market capacity.

### Lesson for Egypt

Keep physical distribution and make it even more diagnosable. The player should be able to trace a missing household good all the way back to the broken supply-chain stage.

---

## 10. Walkers — one of the best ideas and one of the biggest weaknesses

Pharaoh makes the city visually legible by representing many services as people walking through it.

Two broad behaviours are important to understand:

### Destination walkers

These have a known target and route toward it. Examples include freight deliveries, gathering workers and other task-directed agents.

### Roaming/service walkers

These leave a service building and wander along the road network, providing service to buildings they pass. Their route selection around intersections is deliberately limited and can be unpredictable.

Reference service walkers include people such as:

- bazaar sellers;
- water carriers;
- priests;
- physicians;
- dentists;
- morticians;
- herbalists;
- entertainers;
- teachers/librarians;
- constables;
- architects/fire marshals and related civil servants.

### Roadblocks

Roadblocks were introduced so players could constrain roamers. Destination walkers can generally pass them, while ordinary roaming walkers turn around.

The design consequence is significant: experienced players often create closed housing loops with few intersections because reliable walker coverage is more important than organic street layouts.

### Lesson for Egypt

**Keep visible agents. Remove arbitrary roaming as the fundamental service-delivery rule.**

A service agent should understand its job and choose sensible targets while remaining constrained by real roads, distance, congestion, service capacity and travel time.

---

## 11. Employment and recruiters — reference weakness

In the original system, workers do not literally commute from their individual homes to workplaces. A building needing employees can dispatch a recruiter/citizen walker. If that walker reaches populated housing, the building can draw labour from the wider unemployed workforce pool.

This creates artificial behaviour: a remote industrial area may need a tiny housing pocket nearby simply so recruiters can 'touch' housing, even though the recruited workers are not actually represented travelling from those homes.

*Pharaoh: A New Era* later offered a global workforce option that removes much of this friction.

### Lesson for Egypt

Do not preserve the recruiter fiction. Employment should use a genuine labour/commuting model appropriate to whatever citizen granularity we ultimately choose.

---

## 12. Population, immigrants and scribes

City sentiment affects immigration. If conditions are good and housing space exists, new people arrive; bad conditions can drive people away.

A particularly interesting mechanic is the transition to scribal/elite housing. Scribes do not contribute to the ordinary manual workforce, so evolving too much housing upward can create a labour shortage.

### Lesson for Egypt

Social progression should create trade-offs. A richer city should not simply be 'the same economy with larger numbers'. Education, elite classes, administration and labour availability can interact.

---

## 13. Health and sanitation

Reference health systems include:

- wells;
- water supplies and water carriers;
- physicians;
- apothecaries/herbalists;
- dentists;
- mortuaries/morticians;
- food quality/diversity;
- city health risk;
- disease/plague;
- malaria risk in relevant areas.

Mortuaries consume linen, connecting health/funerary service to industry.

### Lesson for Egypt

Service systems are strongest when they depend on the wider economy rather than operating as isolated coverage circles.

---

## 14. Education

Two important reference institutions are Scribal Schools and Libraries. Both consume papyrus, which ties education directly to the reed/papyrus industry.

Education supports higher housing and the emergence of scribes.

### Lesson for Egypt

Keep the principle that institutions consume real goods. A school/library should not be a free perpetual aura after construction.

---

## 15. Entertainment

Reference entertainment includes:

- juggler schools and booths;
- conservatories/musicians and bandstands;
- dance schools/dancers and pavilions;
- Senet Houses supplied with beer;
- Cleopatra's Zoo, requiring animal-related supplies.

Performers are trained, travel to venues and then provide entertainment coverage. Higher housing demands richer/more varied entertainment.

### Lesson for Egypt

Entertainment can be both a service network and visible city life. Avoid reducing it to a single generic 'entertainment score'.

---

## 16. Civil administration, tax and crime

Reference civic systems include:

- palaces;
- tax collectors;
- courthouses/magistrates;
- constables;
- city sentiment;
- wages;
- taxation rate;
- crime;
- theft from civic treasury buildings;
- fire risk;
- structural collapse risk;
- architects and fire prevention services;
- beautification;
- bridges/ferries/water crossings;
- roadblocks.

Taxes depend strongly on housing quality, making residential evolution economically meaningful.

### Lesson for Egypt

Economic success should emerge from the quality and organisation of the city, not merely from placing revenue buildings.

---

## 17. Religion and the five principal gods

The original game uses five principal gods:

- **Osiris** — Nile/flood/agriculture;
- **Ra** — sun/kingdom;
- **Ptah** — industry/workers;
- **Seth** — war;
- **Bast** — home/domestic life.

Some maps recognise only a subset. A patron god generally expects greater devotion.

Reference religious structures/systems include:

- shrines;
- temples;
- priests;
- temple complexes;
- altars;
- oracles;
- festival squares;
- festivals;
- blessings and punishments related to a god's domain.

### Weakness

Much of long-term religious management can become a building-count/coverage obligation: construct enough temples and keep deity sentiment out of danger.

### Lesson for Egypt

Preserve the strong cultural identity and interaction between divine domains and civic systems, but make religion feel like actual civic/religious life rather than temple spam.

---

## 18. Monument construction

Monuments are central to Pharaoh's identity.

Reference monument types across Pharaoh/Cleopatra include:

- mastabas;
- stepped pyramids;
- larger stepped pyramid complexes;
- bent/true pyramids;
- Great Pyramid-style complexes;
- mudbrick pyramids;
- sphinxes;
- sun temples;
- mausoleums;
- obelisks;
- royal burial tombs;
- Abu Simbel-style monumental works;
- Great Library of Alexandria;
- Lighthouse of Alexandria;
- Caesareum-type monumental structures.

### Reference construction logic

Monuments are not paid for and spawned instantly.

They can require:

- site preparation;
- vast material stockpiles;
- quarries or imports;
- storage yards;
- Work Camp peasant gangs;
- sledges/material hauling;
- Stonemasons' Guilds;
- Bricklayers' Guilds;
- Carpenters' Guilds;
- Cleopatra Artisan Guilds for specialised finishing;
- burial provisions for tombs.

Different projects need different combinations of labour and guilds. Large monuments can take years.

### Lesson for Egypt

This is one of the systems to preserve most strongly in spirit and then deepen. Construction should be a visible transformation of the landscape involving logistics, labour and specialist work.

---

## 19. Military and defence

Military is not the primary reason Pharaoh remains memorable, but it is part of the full reference system.

Reference land forces include:

- archers;
- infantry/spearmen supplied through weapons production;
- charioteers supplied through chariot manufacture;
- recruiters;
- forts;
- military academies;
- unit experience/veterancy.

Reference defence/naval systems include:

- walls;
- gatehouses;
- towers;
- warships/naval construction;
- transport across water;
- external requests for military aid;
- invasions tied to scenarios or political failure.

Seth's religious effects can interact with military quality.

### Lesson for Egypt

Military can exist, but it should serve the city-builder rather than turn the game into an RTS. Final combat design remains TBD.

---

## 20. Trade and the wider world

The world map connects settlements and trade partners.

Reference trade concepts include:

- opening land or river/sea trade routes;
- importing resources unavailable locally;
- exporting surplus goods;
- commodity-specific trade partners;
- prices and trade capacity;
- requests from other cities or the Pharaoh;
- kingdom reputation affected by cooperation and failure;
- military requests in addition to goods requests.

Trade often solves geographic problems: a city without clay, wood, stone or food variety must specialise and buy what it lacks.

### Lesson for Egypt

Maps should have economic identities. Geography should create trade relationships, not merely cosmetic differences.

---

## 21. Campaign structure and family history

The original campaign spans a broad historical progression, roughly from the Predynastic period through later ancient Egyptian eras. Missions are grouped into historical periods and generally introduce systems incrementally.

The player maintains a family/dynasty identity across missions. Completed history can be revisited, though the actual built cities are not persistent economic actors in later missions.

Mission briefings define local conditions and goals. Some points offer choices between different mission types.

### Lesson for Egypt

The family/dynasty framing is strong. Our own spin should push persistence further so earlier settlements can potentially remain meaningful in the wider campaign economy.

---

## 22. Mission goals, ratings and scoring

Reference mission goals frequently involve combinations of:

- population;
- prosperity;
- culture;
- monument completion;
- kingdom/reputation standing;
- treasury/economy;
- survival of attacks;
- fulfilment of external requests.

The original also produces an end-of-mission score based on performance factors such as speed, difficulty, ratings, population, treasury and monuments.

### Lesson for Egypt

Goals should reflect the identity of a settlement rather than every map being a generic population/ratings checklist.

---

## 23. Overlays, overseers and information architecture

Pharaoh contains a large amount of information, but it tries to expose it through:

- contextual info-clicks on buildings/people;
- specialist Overseers;
- map overlays;
- message systems;
- city ratings;
- world map;
- monument status/foreman information.

Reference overlays cover systems such as water, fire, damage/collapse, health, desirability, tax, services and other coverage.

### Lesson for Egypt

This is an area where a modern game can dramatically improve. The player should not merely see that a house lacks pottery; they should be able to trace *why*.

---

## 24. Failure, risk and city instability

The original city can fail through more than lack of money.

Reference risks include:

- fire;
- structural collapse;
- disease/plague;
- malaria;
- crime;
- unemployment;
- labour shortages;
- food shortages;
- failing flood/fertility conditions;
- angry gods;
- debt;
- loss of kingdom reputation;
- invasion;
- disrupted trade;
- housing devolution;
- emigration.

### Lesson for Egypt

Failures should have visible causal chains and recoverable states where appropriate. The game should punish poor systems, not hide failures behind mysterious dice rolls.

---

## 25. Cleopatra expansion lessons

Cleopatra is valuable because it shows how to extend the base simulation without replacing it.

Useful reference ideas include:

- specialised burial-tomb construction;
- paints, lamps and funerary provisions;
- Artisan Guilds;
- grave robbery;
- new monument types;
- zoos;
- more unusual scenario pressures;
- tighter time limits or specialised objectives.

### Lesson for Egypt

New content is strongest when it plugs into existing systems and changes logistics rather than merely adding another decorative building.

---

## 26. What Pharaoh did especially well

These are the reference strengths we want to preserve **in spirit**:

1. Visible people and goods make the city understandable.
2. Housing evolution turns service delivery into progression.
3. Production chains interlock instead of remaining isolated.
4. The Nile makes the setting mechanically meaningful.
5. Seasonal workers connect farming and monuments.
6. Monuments are projects rather than purchases.
7. Trade makes geography matter.
8. The campaign gradually introduces complexity.
9. Egyptian religion is integrated into city management.
10. Strong audio/visual atmosphere makes even mundane logistics pleasurable to watch.
11. City problems are spatial: distance and road layout matter.
12. The player is constantly balancing labour, money, food, goods and prestige rather than maximising a single resource.

---

## 27. What aged poorly or became artificial

These are the major reference weaknesses we should not blindly inherit:

1. Random/general walker pathing makes intersections a liability and encourages artificial closed loops.
2. Roadblocks become mandatory optimisation tools instead of optional traffic/service controls.
3. Recruiter-based labour access creates fake slum pockets beside remote industry.
4. Workers are not true commuters.
5. Service coverage can fail for reasons that are difficult to diagnose.
6. Temple and cultural-building requirements can become repetitive building-count exercises.
7. Some systems are represented by global ratios rather than believable local participation.
8. Optimal residential blocks can become formulaic.
9. Military controls are comparatively crude.
10. Earlier campaign cities do not become persistent settlements in a living regional economy.
11. Modern hardware could support deeper simulation and larger populations than the original engine could reasonably attempt.

---

## 28. Reference-to-Egypt translation

| Pharaoh reference idea | Egypt direction |
| --- | --- |
| Roaming walkers | Purposeful service agents with real targets |
| Recruiters touching housing | Real labour availability / commuting |
| Bazaars | Physical local markets and distribution |
| Housing evolution | Original household/neighbourhood progression |
| Nile flood cycle | More visible and mechanically connected hydrology/agriculture |
| Work Camps | Explicit seasonal labour reallocation |
| Temple count/deity mood | More meaningful participation, institutions and festivals |
| Monument construction | Deeper staged construction logistics |
| Storage yards | Rich player-controlled logistics/storage policy |
| Overlays | Causal diagnostics and traceable bottlenecks |
| Mission campaign | Persistent dynasty/world where appropriate |
| Static scenario economy | Settlements with long-term regional roles |
| Scribes | Broader social-class / educated-workforce trade-offs |
| Fixed loops | Organic city layouts should remain viable |

---

## 29. Research sources retained for future work

Primary/reference sources consulted during initial research:

- **Original Pharaoh manual**, Impressions Games / Sierra, 1999. Particularly useful sections: Housing; People and Employment; Farming and Food Production; Industry; Commerce and Trade; Municipal Functions; Religion; Monuments; Health; Entertainment; Education; Military; Ratings; Managing Your City; Designer's Notes; Building Summary.
- **StrategyWiki Pharaoh reference pages** for housing, industry, health, education, religion, entertainment, military, monuments and civil service.
- **Pharaoh: A New Era** community/documentation discussions, especially comparison of classic recruiter behaviour, global labour and roadblocks.
- Contemporary and retrospective reviews of Pharaoh and Cleopatra.
- Long-running Impressions Games community discussions on housing blocks, walkers, labour, roads and culture.
- **Akhenaten** open-source reimplementation as a useful behavioural research reference. It is not our architecture and its code must not be copied without explicit licensing/technical review.

Original manual URL retained for research convenience:

`https://cdn.akamai.steamstatic.com/steam/apps/564530/manuals/Pharaoh_-_manual.pdf`

---

## 30. Research questions still open

Further reference research is still useful for:

- exact original trade-route capacity and pricing behaviour;
- detailed walker lifetime/routing edge cases;
- original mission scripting/event architecture;
- exact flood timing and fertility formulae;
- exact monument stage/resource logic by monument type;
- original crime/sentiment formulas;
- combat and naval AI;
- culture/prosperity/kingdom rating formulas;
- map/editor data structures;
- Cleopatra-specific late-game systems;
- useful lessons from later Impressions city builders such as *Zeus* and *Emperor* without allowing them to dilute Egypt's identity.

These details are valuable for understanding historical design choices, but **Egypt does not need compatibility with Pharaoh**.

---

## 31. Core conclusion

The target is not:

> Make Pharaoh again.

The target is:

> Understand exactly why Pharaoh's city felt alive, retain the best principles, remove the artificial limitations, and build an original Egyptian city simulation that could only reasonably be made with modern hardware and modern design knowledge.
