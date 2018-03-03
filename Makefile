all: .unzip .power

.power:
	make -C tests/power

demo: .unzip .power
	sudo ruby runner.rb -t 600 ./siteList.csv #run for 10 min

install: .unzip .power
	sudo apt-get install python-dev python-pip python-matplotlib python-tk google-chrome-stable nodejs npm
	sudo -H pip install --upgrade pip
	sudo pip install setuptools psutil 
	sudo pip install psrecord matplotlib 
	sudo npm install -g chrome-har-capturer

.unzip:
	make -C tests/interference

clean:
	make clean -C tests/power
	make clean -C tests/interference
