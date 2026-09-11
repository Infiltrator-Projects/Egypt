extends Control

const GOLD := Color("d9b15f")
const PALE_GOLD := Color("f2d79a")
const DESERT := Color("b98347")
const DESERT_DARK := Color("70472c")
const NIGHT := Color("17120f")
const NILE := Color("2d7187")
const NILE_LIGHT := Color("58a8b3")

var _screen := "menu"
var _status_label: Label
var _content: VBoxContainer


func _ready() -> void:
	_build_ui()
	queue_redraw()


func _notification(what: int) -> void:
	if what == NOTIFICATION_RESIZED:
		queue_redraw()


func _draw() -> void:
	var s := size
	if s.x <= 0.0 or s.y <= 0.0:
		return

	# First-pass original backdrop. No external or Pharaoh assets are used.
	draw_rect(Rect2(Vector2.ZERO, s), NIGHT)
	draw_rect(Rect2(0, s.y * 0.54, s.x, s.y * 0.46), DESERT_DARK)
	draw_rect(Rect2(0, s.y * 0.68, s.x, s.y * 0.32), DESERT)

	# Nile ribbon running toward the foreground.
	var river_x := s.x * 0.72
	var river_w := maxf(90.0, s.x * 0.105)
	var river := PackedVector2Array([
		Vector2(river_x - river_w * 0.15, s.y * 0.44),
		Vector2(river_x + river_w * 0.55, s.y * 0.44),
		Vector2(river_x + river_w * 0.35, s.y),
		Vector2(river_x - river_w * 0.55, s.y),
	])
	draw_colored_polygon(river, NILE)
	draw_polyline(
		PackedVector2Array([
			Vector2(river_x + river_w * 0.20, s.y * 0.47),
			Vector2(river_x, s.y)
		]),
		NILE_LIGHT,
		3.0,
		true
	)

	_draw_pyramid(Vector2(s.x * 0.12, s.y * 0.68), s.y * 0.25)
	_draw_pyramid(Vector2(s.x * 0.27, s.y * 0.68), s.y * 0.18)
	_draw_pyramid(Vector2(s.x * 0.38, s.y * 0.68), s.y * 0.13)

	var stars := [
		Vector2(0.08, 0.11), Vector2(0.16, 0.19), Vector2(0.28, 0.09),
		Vector2(0.42, 0.17), Vector2(0.57, 0.08), Vector2(0.66, 0.21),
		Vector2(0.81, 0.12), Vector2(0.91, 0.20)
	]
	for p: Vector2 in stars:
		draw_circle(Vector2(s.x * p.x, s.y * p.y), 1.7, PALE_GOLD)


func _draw_pyramid(base: Vector2, height: float) -> void:
	var half_width := height * 0.78
	var pts := PackedVector2Array([
		Vector2(base.x - half_width, base.y),
		Vector2(base.x, base.y - height),
		Vector2(base.x + half_width, base.y),
	])
	draw_colored_polygon(pts, Color("3b2b22"))
	draw_polyline(
		PackedVector2Array([pts[0], pts[1], pts[2]]),
		Color(0.70, 0.52, 0.31, 0.35),
		2.0,
		true
	)


func _build_ui() -> void:
	var safe := MarginContainer.new()
	add_child(safe)
	safe.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	safe.add_theme_constant_override("margin_left", 64)
	safe.add_theme_constant_override("margin_right", 64)
	safe.add_theme_constant_override("margin_top", 42)
	safe.add_theme_constant_override("margin_bottom", 42)

	_content = VBoxContainer.new()
	_content.add_theme_constant_override("separation", 14)
	safe.add_child(_content)

	_show_menu()


func _clear_content() -> void:
	for child in _content.get_children():
		_content.remove_child(child)
		child.queue_free()


func _show_menu() -> void:
	_screen = "menu"
	_clear_content()

	var spacer := Control.new()
	spacer.custom_minimum_size = Vector2(1, 52)
	_content.add_child(spacer)

	var title := Label.new()
	title.text = "EGYPT"
	title.add_theme_font_size_override("font_size", 72)
	title.add_theme_color_override("font_color", GOLD)
	_content.add_child(title)

	var subtitle := Label.new()
	subtitle.text = "A living city on the Nile"
	subtitle.add_theme_font_size_override("font_size", 22)
	subtitle.add_theme_color_override("font_color", PALE_GOLD)
	_content.add_child(subtitle)

	var rule := ColorRect.new()
	rule.color = Color(GOLD.r, GOLD.g, GOLD.b, 0.70)
	rule.custom_minimum_size = Vector2(410, 2)
	rule.size_flags_horizontal = Control.SIZE_SHRINK_BEGIN
	_content.add_child(rule)

	var gap := Control.new()
	gap.custom_minimum_size = Vector2(1, 28)
	_content.add_child(gap)

	var button_box := VBoxContainer.new()
	button_box.custom_minimum_size = Vector2(340, 0)
	button_box.size_flags_horizontal = Control.SIZE_SHRINK_BEGIN
	button_box.add_theme_constant_override("separation", 10)
	_content.add_child(button_box)

	var new_game := _make_button("New Game")
	new_game.pressed.connect(_start_game_stub)
	button_box.add_child(new_game)

	var continue_game := _make_button("Continue")
	continue_game.disabled = true
	continue_game.tooltip_text = "No saved city yet"
	button_box.add_child(continue_game)

	var settings := _make_button("Settings")
	settings.pressed.connect(_show_settings_stub)
	button_box.add_child(settings)

	var quit := _make_button("Quit")
	quit.pressed.connect(func() -> void: get_tree().quit())
	button_box.add_child(quit)

	_status_label = Label.new()
	_status_label.text = "Pre-alpha  •  First playable shell"
	_status_label.add_theme_font_size_override("font_size", 14)
	_status_label.add_theme_color_override("font_color", Color(0.90, 0.82, 0.68, 0.72))
	_content.add_child(_status_label)


func _make_button(text_value: String) -> Button:
	var button := Button.new()
	button.text = text_value
	button.custom_minimum_size = Vector2(340, 48)
	button.alignment = HORIZONTAL_ALIGNMENT_LEFT
	button.add_theme_font_size_override("font_size", 20)
	button.add_theme_color_override("font_color", PALE_GOLD)
	button.add_theme_color_override("font_hover_color", Color.WHITE)
	button.add_theme_color_override("font_pressed_color", GOLD)
	return button


func _start_game_stub() -> void:
	_screen = "game"
	_clear_content()

	var header := HBoxContainer.new()
	header.add_theme_constant_override("separation", 18)
	_content.add_child(header)

	var title := Label.new()
	title.text = "Settlement on the Nile"
	title.add_theme_font_size_override("font_size", 32)
	title.add_theme_color_override("font_color", GOLD)
	header.add_child(title)

	var flex := Control.new()
	flex.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	header.add_child(flex)

	var back := Button.new()
	back.text = "Return to Main Menu"
	back.pressed.connect(_show_menu)
	header.add_child(back)

	var info := Label.new()
	info.text = "Population  0     Treasury  5000     Year  1     Flood forecast  —"
	info.add_theme_font_size_override("font_size", 16)
	info.add_theme_color_override("font_color", PALE_GOLD)
	_content.add_child(info)

	var placeholder := PanelContainer.new()
	placeholder.size_flags_vertical = Control.SIZE_EXPAND_FILL
	placeholder.custom_minimum_size = Vector2(0, 480)
	_content.add_child(placeholder)

	var map := Control.new()
	map.set_script(preload("res://src/map_stub.gd"))
	map.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	map.size_flags_vertical = Control.SIZE_EXPAND_FILL
	placeholder.add_child(map)

	var hint := Label.new()
	hint.text = "First milestone: game shell active. Roads and simulation are deliberately not implemented yet."
	hint.add_theme_font_size_override("font_size", 15)
	hint.add_theme_color_override("font_color", Color(0.92, 0.86, 0.73, 0.86))
	_content.add_child(hint)


func _show_settings_stub() -> void:
	_status_label.text = "Settings will be implemented after the first city interaction."


func _unhandled_key_input(event: InputEvent) -> void:
	if event is InputEventKey:
		var key_event := event as InputEventKey
		if key_event.pressed and not key_event.echo and key_event.keycode == KEY_ESCAPE and _screen != "menu":
			_show_menu()
