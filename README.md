## eBPF with OpenAPI

1. Install requirement.txt using pip3 install -r requirements.txt

2. Check whether the below mentioned modules are installed. If not install them using sudo apt get install <module name>

Required Modules
* libelf-dev
* gcc-multilib
* g++-multilib

3. Run run.sh file using
sudo bash run.sh

## Working
The bash file (run.sh) will create a python virtual environment and build the eBPF file. Then the python server will be initiated and the eBPF will be added to the server.
