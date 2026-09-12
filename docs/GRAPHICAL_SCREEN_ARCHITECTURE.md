# Egypt — Graphical Screen Architecture

Status: **AUTHORITATIVE PROJECT PHILOSOPHY**

Date: 2026-09-12

This document records the intended way of thinking about Egypt's entire presentation layer. It is not merely a styling preference. It is the model the renderer, UI, asset pipeline and platform boundary should be designed around.

## The core idea

**The screen is the thing.**

Egypt does not begin with widgets, labels, controls or text and then ask how to draw them. It begins with a graphical screen in memory and composes that screen from other graphical surfaces and images.

A menu is a graphical screen.

A HUD is a graphical screen layered over the city screen.

A panel is another graphical screen layered over that.

A dialog is another graphical surface.

A button is a graphical image in one of several visual states.

An icon is a graphical image.

A word is a sequence of graphical glyph images.

A number is a sequence of graphical digit images.

Everything visible to the player is ultimately pixels in game-owned memory.

The program may understand the *meaning* of those elements — for example, that a region is a pause button or that a sequence of glyphs represents a date — but the presentation system does not depend on native controls, native text, terminal rendering, desktop widgets or a separate text mode.

## Graphical screens on graphical screens

The intended composition model is conceptually:

```text
world/city graphical surface
    + HUD graphical surface
        + control-state graphics
        + graphical glyphs
    + inspector graphical surface
        + icons
        + graphical glyphs
    + modal/dialog graphical surface
        + buttons as graphics
        + graphical glyphs
    = completed frame in memory
```

The final result is one complete image. The host/platform layer presents that image.

The important point is that a "window" inside Egypt is not an operating-system window. It is simply another graphical surface composited over the current screen. A "button" is not a toolkit widget. It is artwork with an interactive region. A "label" is not a text object. It is graphical glyph artwork placed into the screen image.

## No text as a presentation primitive

Egypt may use strings internally as data, identifiers, localisation keys or simulation values, but **there is no player-facing text primitive in the visual model**.

If the game needs to show `JAN 3500 BC`, the runtime presentation is not "draw text JAN 3500 BC". The runtime presentation is "compose the graphical glyphs J, A, N, space, 3, 5, 0, 0, space, B, C into this graphical surface".

If the game needs to show `5000`, those digits are graphical assets.

If the game needs to show `MAIN MENU`, that label is graphical imagery, whether it is precomposed as one asset or assembled from a graphical glyph atlas.

A font may be used during asset creation, but the game-facing result is pixel artwork. The runtime renderer should think in terms of images, surfaces, sprite regions, masks, palettes and compositing, not operating-system text rendering.

This distinction matters because a framebuffer alone does not guarantee a graphical visual language. It is possible to build a DOS-looking text UI *inside* a framebuffer by drawing rectangles, lines and enlarged bitmap-font cells. That is specifically what Egypt must avoid.

## Frame ownership and presentation

Egypt owns the frame.

The game prepares a complete next frame in memory. The currently completed frame is presented while the next frame is being assembled in another buffer or equivalent back surface. When the next image is ready, presentation swaps to that completed image.

Conceptually:

```text
front buffer -> complete image currently visible
back buffer  -> next complete image being composed

compose entire next screen
swap/present
repeat
```

The exact implementation may use software buffers, mapped presentation memory, texture upload, page flipping, DRM/KMS, Win32 presentation or another host-specific method. Those details are outside the game core. They do not change the rule that Egypt itself owns and composes the visible image.

The operating system is not responsible for drawing Egypt's player-facing controls, words, panels or menus.

## Artwork first, interaction second

The default design question is not:

> What widget should this be?

or:

> Which primitive should draw this control?

The default question is:

> What should this part of the finished screen look like, and which graphical asset or graphical surface represents it?

Interaction is then attached to the relevant screen region or graphical object.

This intentionally reverses the normal modern GUI mentality. Visual composition is primary. Widget semantics are secondary.

## Why this approach is preferred for Egypt

For this project, this model is considered superior to conventional widget-and-font UI because it gives the game total control over its visual language.

It prevents operating-system styling, terminal-like typography and generic toolkit conventions from leaking into the game. The same graphical screen can look the same on Linux, Windows or another host because the game is presenting its own pixels rather than asking each platform to construct its interface.

It also makes the whole game visually coherent. Terrain, buildings, citizens, HUD frames, labels, buttons, numbers and dialog surfaces all belong to the same authored world. There is no conceptual boundary where "the game graphics stop and the computer UI begins".

This is the quality Shannon associates with classic Amiga-era software and games: even the desktop and utility surfaces were graphical compositions. The user did not feel as though a command-line program had been dressed with boxes and labels. The screen itself was designed.

For Egypt, that means visual identity is built into the pixels rather than delegated to a UI framework.

## Consequences for Egypt's renderer and asset pipeline

The renderer and asset system should naturally support:

- complete off-screen graphical surfaces;
- fast image/sprite blitting;
- transparent/masked graphical elements;
- graphical state variants for controls;
- graphical glyph atlases for dynamic words and numbers;
- reusable panel, frame, ornament and icon assets;
- composition of one graphical surface over another;
- clipping regions and dirty-region optimisation where useful;
- scaling rules that preserve intentional pixel/art quality;
- front/back-buffer or equivalent completed-frame presentation.

The runtime should make asset composition easy enough that nobody is tempted to recreate finished UI with debug rectangles and bitmap-font calls merely because those are convenient.

## What procedural drawing is still for

Low-level raster primitives are still valid engine tools. They are useful for terrain rasterisation, masks, selection outlines, placement previews, debug overlays, collision visualisation, temporary scaffolding and other genuinely procedural imagery.

They are not the intended substitute for authored player-facing interface artwork.

A procedurally generated river, cloud shadow or terrain blend is still graphics because the output is graphical imagery. The prohibition is against using programmer-oriented UI primitives and text rendering as the finished visual language of the game.

## Review test

When reviewing a player-facing screen, ask:

> If I did not know how this was programmed, would this look like a deliberately drawn graphical screen, or would it look like text and controls placed on top of a framebuffer?

If it looks like the latter, the implementation has drifted away from this architecture.

A second useful test is:

> Could this element have come from a terminal, native desktop toolkit or debug overlay with only a colour change?

If yes, it is probably not finished Egypt presentation artwork.

## Non-negotiable examples

The following are not acceptable as final player-facing presentation:

- enlarged 5x7 or similar debug bitmap fonts;
- native operating-system controls;
- system font rendering used as the final in-game look;
- generic rectangle-plus-label buttons;
- geometric pseudo-ornament standing in for authored artwork;
- panels that are merely flat colour fills with a border when the intended design calls for illustrated UI;
- a HUD that visually reads as a DOS utility, terminal application, debug tool or desktop form.

The target is not "a UI drawn with a graphics API".

The target is **a graphical world made from graphical screens on graphical screens, with no separate player-visible text mode at all**.
