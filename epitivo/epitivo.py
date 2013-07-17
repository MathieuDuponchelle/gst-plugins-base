from gi.repository import GLib
from gi.repository import GObject
from gi.repository import Gst
from gi.repository import Gtk

from gettext import gettext as _

from pipeline import SimplePipeline, Seeker
from viewer import PitiviViewer

def unlink_it(pad, info, pipeline):
    tee = pipeline.get_by_name("t")
    pad.set_active(False)
    tee.release_request_pad(pad)
    return Gst.PadProbeReturn.REMOVE

def restart_pipeline():
    new_pipeline = Gst.parse_launch("filesrc location=test.ogv ! matroskademux ! epitechdec ! videoconvert ! xvimagesink")
    new_pipeline.set_state(Gst.State.PLAYING)
    return False

def clickedCb(button, pipeline):
    GObject.timeout_add(1000, restart_pipeline)

class Epitivo:
    def __init__(self):
        self._createUi()
        self._createPipeline()

    def goToEnd(self):
        if not self.live:
            self.live = True
            self.replay_viewer.hide()
            self.viewer.show()

    def togglePlayback(self):
        if self.live:
            if self.pipeline.getState() != Gst.State.PLAYING:
                self.pipeline.togglePlayback()
            elif self.replay_pipeline and self.replay_pipeline.getState() == Gst.State.PAUSED:
                self.expectedState = Gst.State.PLAYING
                self._maybeStartPipeline(self.pipeline.getPosition() - 2 * Gst.SECOND)
            else:
                self.expectedState = Gst.State.PAUSED
                self._maybeStartPipeline(self.pipeline.getPosition())
        else:
            if self.replay_pipeline and self.replay_pipeline.getState() == Gst.State.PAUSED:
                self.expectedState = Gst.State.PLAYING
                self._maybeStartPipeline(self.replay_pipeline.getPosition())
            else:
                self.expectedState = Gst.State.PAUSED
                self._maybeStartPipeline(self.replay_pipeline.getPosition())

    def _createUi(self):
        self.window = Gtk.Window()
        self.undock_action = Gtk.Action("WindowizeViewer", _("Undock Viewer"),
            _("Put the viewer in a separate window"), None)
        self.viewer = PitiviViewer(self, undock_action=self.undock_action)
        self.replay_viewer = PitiviViewer(self, undock_action=self.undock_action)
        vbox = Gtk.VBox()
        vbox.pack_start(self.viewer, False, False, False)
        self.slider = Gtk.Scale.new(Gtk.Orientation.HORIZONTAL, None)
        self.slider.set_draw_value(False)
        self.window.add(vbox)
        self.window.connect("delete-event", self._exit)
        self.window.show_all()
        self.replay_viewer.hide()
        vbox.pack_start(self.replay_viewer, False, False, False)
        vbox.pack_start(self.slider, False, False, False)
        self.slider.show()

    def _createPipeline(self):
        pipeline = Gst.parse_launch("v4l2src ! video/x-raw, width=320, height=240, format=RGB ! tee name=t ! queue name=queue1 ! videoconvert ! xvimagesink name=sink_bill_gates sync=false t. ! epitechenc ! matroskamux ! filesink location=test.ogv")
        self.pipeline = SimplePipeline(pipeline, pipeline.get_by_name("sink_bill_gates"))
        self.window.connect("draw", self._setPipelineOnViewer)
        self.replay_pipeline = None
        self.live = True

    def _exit(self, window, event):
        if self.replay_pipeline:
            self.replay_pipeline.release()
        self.pipeline.release()
        Gtk.main_quit()

    def _setPipelineOnViewer(self, widget, cr):
        self.viewer.setPipeline(self.pipeline)
        self.seeker = Seeker()
        self.pipeline.connect("position", self._positionCb)
        self.pipeline.activatePositionListener()
        self.window.disconnect_by_func(self._setPipelineOnViewer)
        self.slider.connect("change-value", self._seekThat)

    def _maybeStartPipeline(self, value):
        self.live = False
        if self.replay_pipeline:
            self.replay_pipeline.setState(self.expectedState)
            self.viewer.hide()
            self.replay_viewer.show()
            self.replay_pipeline._pipeline.seek_simple(Gst.Format.TIME, Gst.SeekFlags.FLUSH, value)
            return
        self.viewer.hide()
        pipeline = Gst.parse_launch("filesrc location=test.ogv ! matroskademux ! epitechdec ! videoconvert ! xvimagesink name=sink_bill_gates")
        self.replay_viewer.show()
        self.replay_viewer.connect("draw", self._setReplayPipelineOnReplayViewer, value)
        self.replay_pipeline = SimplePipeline(pipeline, pipeline.get_by_name("sink_bill_gates"))

    def _seekThat(self, scale, scroll_type, value):
        self.expectedState = Gst.State.PLAYING
        self._maybeStartPipeline(value)

    def _setReplayPipelineOnReplayViewer(self, widget, cr, value):
        self.replay_viewer.setPipeline(self.replay_pipeline)
        self.valueSet = False
        self.replay_pipeline.connect("state-change", self._pipelineStateChangedCb, value)
        self.replay_viewer.disconnect_by_func(self._setReplayPipelineOnReplayViewer)
        self.replay_pipeline.setState(self.expectedState)
#        self.replay_pipeline.connect("position", self._positionCb)
#        self.replay_pipeline.activatePositionListener()

    def _pipelineStateChangedCb(self, pipeline, state, value):
        if (state == Gst.State.PLAYING or state == Gst.State.PAUSED) and not self.valueSet:
            self.replay_pipeline._pipeline.seek_simple(Gst.Format.TIME, Gst.SeekFlags.FLUSH, value)
            self.valueSet = True

    def _positionCb(self, unused_pipeline, position):
        self.slider.set_range(0, position)
        if (self.live):
            self.slider.set_value(position)
        else:
            self.slider.set_value(self.replay_pipeline.getPosition())

if __name__ == "__main__":
    Gst.init([])
    Gtk.init([])
    GObject.threads_init()
    Epitivo()
    Gtk.main()
