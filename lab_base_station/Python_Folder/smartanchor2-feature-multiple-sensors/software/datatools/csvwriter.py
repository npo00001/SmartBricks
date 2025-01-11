import csv
import boto3
from botocore.exceptions import NoCredentialsError
from os import makedirs, path, fsync

def _get_file(file_path):
    makedirs(path.dirname(file_path), exist_ok=True)
    file = open(file_path, 'a', newline='')
    csvwriter = csv.writer(file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    return file, csvwriter

class CSVWriter:
    def __init__(self, file_path, s3_bucket='smartbricksbucket', s3_key_prefix=""):
        self.file, self.writer = _get_file(file_path)
        self.s3_bucket = s3_bucket
        self.s3_key_prefix = s3_key_prefix
        self.s3_client = boto3.client('s3') if s3_bucket else None

    def close(self):
        self.file.close()

    def write(self, data):
        """Writes data to file with basic csv formatting. Flushes file buffer immediately after.
           Also uploads the file to S3 if S3 is configured."""
        
        # Write data to local file
        self.writer.writerow(data)
        
        # Flush file buffer
        self.file.flush()
        fsync(self.file.fileno())
        
        # Upload to S3 if S3 configuration is provided
        if self.s3_client:
            s3_key = f"{self.s3_key_prefix}/{self.file.name.split('/')[-1]}"
            try:
                self.s3_client.upload_file(self.file.name, self.s3_bucket, s3_key)
                print(f"Uploaded {self.file.name} to s3://{self.s3_bucket}/{s3_key}")
            except NoCredentialsError:
                print("AWS credentials not available for S3 upload.")

    def set_file(self, file_path):
        """Change the file this object will write to."""
        
        self.close()  # Close the previous file
        self.file, self.writer = _get_file(file_path)  # Open a new file and update the csvwriter
