## Guide to Run Project
NILM processing software and firmware tools

### Create .env
Create .env file in root directory of project and place the following 
```
DATA_FOLDER_LOCATION = "../../data/dataDB/"
PREPROCESS_FOLDER_LOCATION = "../data/dataDB/PREPROCESS"
METERING_FOLDER_LOCATION="../data/dataDB/METERING"
PROBLEM_FILES="../data/dataDB/FILE_ERRORS"
CSV_CONCAT="../data/dataDB/CSV_CONCAT"

AWS_ACCESS_KEY_ID=
AWS_SECRET_ACCESS_KEY=
AWS_REGION=
```
Feel free to set parameteters

###  Create virtual environment and install requirements
After creating virtual environment, do a pip install using the command below:
```
pip install -r requirements.txt
```
### .gzip to plot
Make sure the .env file is correctly updated
Run the txt2csv.py
Run merge_csv.py
Run example.py (change list indexing to appropriate values)

It should plot a figure with Voltage, Current, Power and RMS Power

### Upload to AWS bucket
Make sure the correct variables for the AWS access are set the .env file (speak to Peals if you need access)
Run upload_bucket.py - Make sure filenames are without blankspaces and check if you want to upload all files 
For firmware additional support contact Diogo Pedroso or Diogo Brito


[DATALOGGER Firmware](datalogger/README.md)

[NILM Firmware](nilm/README.md)

[RaspiWiFi Software](RaspiWiFi/README.md)