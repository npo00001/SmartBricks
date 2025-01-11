import csv

from os import makedirs, path, fsync


def _get_file(file_path):
    makedirs(path.dirname(file_path), exist_ok=True)
    file = open(file_path, 'a', newline='')
    csvwriter = csv.writer(file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    return file, csvwriter


class CSVWriter:
    def __init__(self, file_path):
        self.file, self.writer = _get_file(file_path)

    def close(self):
        self.file.close()

    def write(self, data):
        """Writes data to file with basic csv formatting. Flushes file buffer immediately after."""

        # write data to file, in csv format
        self.writer.writerow(data)

        # flush file buffer
        self.file.flush()
        fsync(self.file.fileno())

    def set_file(self, file_path):
        """Change the file this object will write to."""

        self.file.close()  # close the previous file
        self.file, self.writer = _get_file(file_path)  # open a new file and update the csvwriter
