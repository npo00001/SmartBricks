import matplotlib.pyplot as plt


class Line:
    def __init__(self, xdata, ydata, line):
        self.xdata = xdata
        self.ydata = ydata
        self.line_obj = line


class Plotter:
    def __init__(self, xstart, xrange, yrange, title, xlabel, ylabel, solid_lines=()):
        self.lines = {}

        # plot parameters
        self.X_RANGE = xrange  # range of x-axis
        self.Y_RANGE = yrange  # range of y-axis

        # create figure for plotting
        self.fig = plt.figure()
        self.ax = self.fig.add_subplot(1, 1, 1)
        self.ax.set_ylim(self.Y_RANGE)
        self.ax.set_xlim([xstart - self.X_RANGE, xstart])

        # create warning lines
        for line_val in solid_lines:
            self.ax.axhline(y=line_val, color='r')

        # add plot labels
        self.ax.set_title(title)
        self.ax.set_xlabel(xlabel)
        self.ax.set_ylabel(ylabel)

        plt.ion()

    def plot(self, x, y, label):
        if label not in self.lines:
            # create an initially blank line
            xdata, ydata = [], []
            line, = self.ax.plot(xdata, ydata, label=label)
            self.lines[label] = Line(xdata, ydata, line)
            self.ax.legend(loc="upper left")

        line = self.lines[label]

        # extend line data
        line.xdata.append(x)
        line.ydata.append(y)

        # trim values off the screen to save memory. keep one data point off-screen, so line extends off-screen.
        while len(line.xdata) > 1 and line.xdata[1] < x - self.X_RANGE:
            line.xdata.pop(0)
            line.ydata.pop(0)

        # update line data
        line.line_obj.set_xdata(line.xdata)
        line.line_obj.set_ydata(line.ydata)

        # move window
        self.ax.set_xlim([x - self.X_RANGE, x])

    def update(self, seconds):
        """Give window time to update. Must be run periodically or window will show (Not Responding)."""
        plt.figure(self.fig)
        plt.pause(seconds)

    def done(self):
        """Keep program running to keep window open. If program is done."""
        plt.figure(self.fig)
        plt.show(block=True)
