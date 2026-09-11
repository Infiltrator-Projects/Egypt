extends Control

const SAND_A := Color("c59a62")
const SAND_B := Color("b88955")
const NILE := Color("2e768d")
const FERTILE := Color("6e7750")
const GRID := Color(0.18, 0.14, 0.10, 0.18)


func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	resized.connect(func() -> void: queue_redraw())
	queue_redraw()


func _draw() -> void:
	var s := size
	if s.x <= 0.0 or s.y <= 0.0:
		return

	draw_rect(Rect2(Vector2.ZERO, s), SAND_A)

	# Schematic terrain texture for the bootstrap screen.
	for y in range(0, int(s.y), 48):
		for x in range(0, int(s.x), 48):
			if (int(x / 48) + int(y / 48)) % 2 == 0:
				draw_rect(Rect2(x, y, 48, 48), Color(SAND_B.r, SAND_B.g, SAND_B.b, 0.16))

	var river_center := s.x * 0.58
	var river_half := maxf(65.0, s.x * 0.08)
	draw_rect(Rect2(river_center - river_half - 24, 0, river_half * 2 + 48, s.y), FERTILE)
	draw_rect(Rect2(river_center - river_half, 0, river_half * 2, s.y), NILE)

	# Grid hints at future build cells without pretending gameplay exists yet.
	for x in range(0, int(s.x) + 1, 32):
		draw_line(Vector2(x, 0), Vector2(x, s.y), GRID, 1.0)
	for y in range(0, int(s.y) + 1, 32):
		draw_line(Vector2(0, y), Vector2(s.x, y), GRID, 1.0)

	# One tiny settlement marker so the first city screen does not feel empty.
	var centre := Vector2(s.x * 0.36, s.y * 0.52)
	draw_rect(Rect2(centre - Vector2(18, 14), Vector2(36, 28)), Color("6d4329"))
	draw_colored_polygon(
		PackedVector2Array([
			centre + Vector2(-22, -14),
			centre + Vector2(0, -34),
			centre + Vector2(22, -14)
		]),
		Color("8c5a32")
	)
