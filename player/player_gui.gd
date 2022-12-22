extends CanvasLayer

var fps_label: Label

func _ready():
	fps_label = get_node("FPSLabel");
	OS.window_size *= 2
	OS.center_window()

func _process(delta: float):
	if fps_label != null:
		var fps = Engine.get_frames_per_second()
		fps_label.text = "FPS %d" % fps;
