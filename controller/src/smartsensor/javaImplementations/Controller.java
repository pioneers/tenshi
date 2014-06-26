// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

import java.util.ArrayList;

public class Controller 
{
	public ArrayList<SmartSensor> sensors;
	public static final int ID_LENGTH = 4;
	
	public static void main(String[] args)
	{
		SmartSensor one = new SmartSensor(14); // 0b1110
		SmartSensor two = new SmartSensor(10); // 0b1010
		SmartSensor three = new SmartSensor(15); // 0b1111
		SmartSensor four = new SmartSensor(9); // 0b1001
		SmartSensor five = new SmartSensor(1); // 0b1001
		SmartSensor six = new SmartSensor(2); // 0b1001
		SmartSensor seven = new SmartSensor(3); // 0b1001
		SmartSensor eight = new SmartSensor(4); // 0b1001
		SmartSensor nine = new SmartSensor(5); // 0b1001
		SmartSensor ten = new SmartSensor(6); // 0b1001
		SmartSensor eleven = new SmartSensor(7); // 0b1001
		SmartSensor tewlve = new SmartSensor(8); // 0b1001
		SmartSensor thirteen = new SmartSensor(9); // 0b1001
		SmartSensor fourteen = new SmartSensor(11); // 0b1001
		SmartSensor fifteen = new SmartSensor(12); // 0b1001
		SmartSensor sixteen = new SmartSensor(13); // 0b1001
		
		ArrayList<SmartSensor> s = new ArrayList<SmartSensor>();
		s.add(one);
		s.add(two);
		s.add(three);
		s.add(four);
		s.add(five);
		s.add(six);
		s.add(seven);
		s.add(eight);
		s.add(nine);
		s.add(ten);
		s.add(eleven);
		s.add(tewlve);
		s.add(thirteen);
		s.add(fourteen);
		s.add(fifteen);
		s.add(sixteen);
		
		Controller master = new Controller(s);
			
		// getToState() test
//		System.out.println(master.getState());
//		master.getToState(1, 1);
//		System.out.println(master.getState());
//		
//		master.reset();
//		System.out.println(master.getState());
//		master.getToState(2, 2);
//		System.out.println(master.getState());
//		
//		master.reset();
//		System.out.println(master.getState());
//		master.getToState(10, 4);
//		System.out.println(master.getState());
		
		ArrayList<Integer> allSensors = master.enumeration(); 
		System.out.println(allSensors.toString());
	}
	
	public Controller(ArrayList<SmartSensor> s)
	{
		sensors = s;
	}

	public ArrayList<Integer> enumeration()
	{
		ArrayList<Integer> knownIDs = new ArrayList<Integer>();
		System.out.println("Next number to try: " + 0 + " " + Integer.toBinaryString(0));
		this.findSensorID(knownIDs, 0, 1);
		System.out.println("Next number to try: " + 1 + " " + Integer.toBinaryString(1));
		this.findSensorID(knownIDs, 1, 1);
		return knownIDs;
	}
	
	public void findSensorID(ArrayList<Integer> knownIDs, int currID,  int bitDepth)
	{
		this.getToState(currID, bitDepth);
		
		if  (!isSomethingThere())
		{
			return;
		}
		
		if (bitDepth == ID_LENGTH)
		{
			knownIDs.add(currID);
			return;
		}

		System.out.println("Next number to try: " + (currID | (1 << bitDepth)) + " " + Integer.toBinaryString(currID | (1 << bitDepth)));
		findSensorID(knownIDs, currID | (1 << bitDepth), bitDepth + 1);
		System.out.println("Next number to try: " + (currID & (0 << bitDepth)) + " " + Integer.toBinaryString(currID & (0 << bitDepth)));
		findSensorID(knownIDs, currID, bitDepth + 1);
	}
	
	public void getToState(int currID, int bitDepth)
	{	
		this.reset();
		int pos = 1;
		int nextBit = getBit(currID, pos); // the next bit in id that we will include
		//System.out.println("getBit function returned " + nextBit + " with id: " + currID + " and with position:" + pos);
		this.turnOff( (~nextBit)&1, 1);
		//System.out.println("gitBit function called turn off here with " + Integer.toBinaryString(~nextBit) + " as the argument");
		int currState = nextBit; // what ID substring we are at, eventually we will get to currID
		
		while (pos < bitDepth)
		{
			pos = pos + 1;
			nextBit = getBit(currID, pos);
//			System.out.println("getBit function returned " + nextBit + " with id: " + currID + " and with position:" + pos);
			int notNextBit = (~nextBit) & 1;
			turnOff(currState | (notNextBit << (pos-1)), pos);
			currState = currState | (nextBit << (pos-1));
		}
	}
	
	public int getBit(int num, int depth)
	{
		String binRep = Integer.toBinaryString(num);
		binRep = SmartSensor.rev(binRep);
		if ( (depth-1) >= binRep.length() || binRep.charAt(depth-1) == '0')
		{
			return 0;
		}
		else if (binRep.charAt(depth-1) == '1')
		{
			return 1;
		}
		else
		{
			throw new IllegalArgumentException();
		}
	}
	
	public void turnOff(int id_num, int bitDepth)
	{
		String bitMask = "";
		for(int i = 0; i < bitDepth; i++)
		{
			bitMask += '1';
		}
		
		for (int i = bitDepth; i <= ID_LENGTH; i++)
		{
			bitMask += '0';
		}

		bitMask = SmartSensor.rev(bitMask);
//		System.out.println("turnOff method in Controller has a bitMask of: " + bitMask);
//		System.out.println("turnOff method in Controller wants to turn off: " + Integer.toBinaryString(id_num) + " " + id_num);
		
		for (SmartSensor s:sensors)
		{
			if (s.isOn())
			{
				s.turnOff(id_num, bitMask);				
			}
		}
	}
	
	public String getState()
	{
		String toRtn = "";
		for(SmartSensor s: this.sensors)
		{
			toRtn += " (" + s.ID + "," + s.isOn() + ") ";
		}
		return toRtn;
	}
	
	public boolean isSomethingThere()
	{
		for (SmartSensor s: this.sensors)
		{
			if (s.isOn())
			{
				return true;
			}
		}
		return false;
	}
	
	public void reset()
	{
		for (SmartSensor s: this.sensors)
		{
			s.reset();
		}
	}
}