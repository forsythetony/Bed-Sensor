
#!/bin/bash

#Constants
JARLOCATION="execs/psbedsensor.0630.jar"
LOGFILE="BedSensor.log"
NEWLOG="wirelessTest.log"

#Coloring stuff
redTxt='\e[0;31m'
greenTxt='\e[0;32m'
cyanTxt='\e[0;36m'
NC='\e[0m'

#Testing stuff
testFlag=0

writeToLog () 
{
	
	if [ -z "$1" ]; then
		echo "Argument supplied to writeToLog was invalid."
	else
		echo $1 >> "${NEWLOG}"
	fi
	
}

# Gather user input
confirmFlag=0
while [ "${confirmFlag}" = 0 ]; do
	
	# Get the IP address from the user
	goodFlag=0
	while [ "x${goodFlag}" = x0 ]; do
		echo -n "Enter the IP address to use: "
		
		
		if [ ${testFlag} = 0 ]; then
			ipAddress="200"
		else
			read ipAddress
		fi
		
		if [[ "x${ipAddress}" =~ [0-9]*\.?[0-9] ]]; then
			echo -e "The IP address ${greenTxt}${ipAddress}${NC} was valid"
			goodFlag=1
		else
			echo "IP address not valid. Let's try that again..."
		fi
	done
	
	# Get the port number from the user
	goodFlag=0
	while [ "x${goodFlag}" = x0 ]; do
		echo -n "Enter the port number to use:  "
		
		
		if [ ${testFlag} = 0 ]; then 
			portNumber="123"
		else
			read portNumber
		fi
		
		if [[ ${portNumber} =~ ^[[:digit:]]{1,9}$ ]]; then
			echo -e "The port number ${greenTxt}${portNumber}${NC} was valid"
			goodFlag=1
		else
			echo "Port number not valid. Let's  try that again."
		fi
	done
	
	echo -e "Is this information correct [y/n]?\n\n IP Address:\t${greenTxt}${ipAddress}${NC}\n Port:\t\t${greenTxt}${portNumber}${NC}\n"
	
	
	if [ ${testFlag} = 0 ]; then
		confirmInfo="y"
	else
		read confirmInfo
	fi
	
	if [[ "x${confirmInfo}" =~ (Yes)|(yes)|[Yy] ]]; then
		confirmFlag=1
	else
		confirmFlag=0
	fi
	
done


# Run the jar file with the provided information

timeout 2 java -jar "${JARLOCATION}" "${ipAddress}" "${portNumber}"

# Read from the bed sensor log file and write to new log file

if [ -e "${LOGFILE}" ]; then
	echo "The file ${LOGFILE} exists."
else
	echo "The file ${LOGFILE} does not exist."
fi

while read line
do
	theLine=( $line )
	
	if [ ${theLine[3]} = "[INFO]" ]; then
		writeToLog "${line}"
	fi
	
done < ${LOGFILE}






