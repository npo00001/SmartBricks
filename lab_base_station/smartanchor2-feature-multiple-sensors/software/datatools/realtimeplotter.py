from matplotlib.dates import date2num
from .plotter import Plotter

SECONDS_PER_DAY = 86_400


class RealTimePlotter(Plotter):
    def __init__(self, xstart, xrange, yrange, title, xlabel, ylabel,
                 x_minor_locator, x_minor_formatter, x_major_locator, x_major_formatter, solid_lines=()):
        super().__init__(date2num(xstart), xrange.total_seconds() / SECONDS_PER_DAY,
                         yrange, title, xlabel, ylabel, solid_lines)

        # format x axis labels
        self.fig.subplots_adjust(bottom=0.15)  # slightly increase space below plot for labels
        self.ax.xaxis.set_minor_locator(x_minor_locator)
        self.ax.xaxis.set_minor_formatter(x_minor_formatter)
        self.ax.xaxis.set_major_locator(x_major_locator)
        self.ax.xaxis.set_major_formatter(x_major_formatter)
        self.ax.xaxis.remove_overlapping_locs = False

    def plot(self, x, y, label):
        super().plot(date2num(x), y, label)
