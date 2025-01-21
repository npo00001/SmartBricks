import csv
from os import makedirs, path, fsync

def _get_file(file_path):
    """
    Ensure the directory exists and open a CSV file with the specified path.
    Returns the file object and a CSV writer object.
    """
    makedirs(path.dirname(file_path), exist_ok=True)
    file = open(file_path, 'a', newline='')
    csv_writer = csv.writer(file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    return file, csv_writer

class CSVWriter:
    def __init__(self, file_path):
        """
        Initialize the CSVWriter with a file path.
        Opens the file and prepares the CSV writer.
        """
        self.file, self.writer = _get_file(file_path)

    def close(self):
        """Close the underlying file."""
        self.file.close()

    def write(self, data):
        """
        Writes data to the CSV file in the proper format.
        Flushes the file buffer immediately after writing.
        """
        self.writer.writerow(data)  # Write data to the file in CSV format
        self.file.flush()  # Flush file buffer
        fsync(self.file.fileno())  # Ensure the data is written to disk

    def set_file(self, file_path):
        """
        Set a new file path for the writer.
        Closes the previous file and updates the writer for the new file.
        """
        self.file.close()  # Close the previous file
        self.file, self.writer = _get_file(file_path)  # Open a new file and update the writer
