import csv
# import boto3
# from botocore.exceptions import NoCredentialsError
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
    def __init__(self, file_path): # s3_bucket='smartbricksbucket', s3_key_prefix=""
        """
        Initialize the CSVWriter with a file path.
        Opens the file and prepares the CSV writer.
        """
        self.file, self.writer = _get_file(file_path)
        # self.s3_bucket = s3_bucket
        # self.s3_key_prefix = s3_key_prefix
        # self.s3_client = boto3.client('s3') if s3_bucket else None

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

        # # Upload to S3 if S3 configuration is provided
        # if self.s3_client:
            # s3_key = f"{self.s3_key_prefix}/{self.file.name.split('/')[-1]}"
            # try:
                # self.s3_client.upload_file(self.file.name, self.s3_bucket, s3_key)
                # print(f"Uploaded {self.file.name} to s3://{self.s3_bucket}/{s3_key}")
            # except NoCredentialsError:
                # print("AWS credentials not available for S3 upload.")

    def set_file(self, file_path):
        """
        Set a new file path for the writer.
        Closes the previous file and updates the writer for the new file.
        """
        self.file.close()  # Close the previous file
        # self.close() # This line was used instead of the line above in the archived code.
        self.file, self.writer = _get_file(file_path)  # Open a new file and update the writer
