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

public class SmartSensor 
{
	public int ID;
	private boolean on;
	
	public static void main(String[] args)
	{
		System.out.println(maskBits("00001111",14));
		System.out.println(maskBits("00000111",14));
		System.out.println(maskBits("00000011",14));
		SmartSensor example = new SmartSensor(206);
		System.out.println(example.turnOff(15, "00001111"));
		System.out.println(example.turnOff(14, "00001111"));
	}
	
	public SmartSensor(int id_num)
	{
		ID = id_num;
		on = true;
	}
	
	public void reset()
	{
		on = true;
	}
	
	public boolean isOn()
	{
		return on;
	}
	
	public boolean turnOff(int id_num, String sigPositions)
	{
		id_num = maskBits(sigPositions, id_num);
		int myID = maskBits(sigPositions, this.ID);
		
//		System.out.println("turnOff function SmartSensor id num: " + id_num + " == myID:" + myID + "?");
		
		if (id_num == myID)
		{
			on = false;
		}
		
//		System.out.println("Sensor " + this.ID + " is on:" + this.on);
		
		return !on;
	}
	
	private static int maskBits(String bitMask, int binNumber)
	{
		String binRep = Integer.toBinaryString(binNumber);
		
//		System.out.println("maskBit function SmartSensor binNumber " + binRep);
//		System.out.println("maskBit function bitMask id num " + bitMask);
		
		bitMask = SmartSensor.rev(bitMask);
		binRep = SmartSensor.rev(binRep);
		
		for (int index = 0; index < bitMask.length() && index < binRep.length(); index++)
		{
			if (bitMask.charAt(index) == '0' && binRep.charAt(index) == '1')
			{
				binNumber = binNumber -  ((int)Math.pow(2.0, index));
			}
		}
		
//		System.out.println("maskBit function new binNumber: " + Integer.toBinaryString(binNumber) + " " + binNumber);
		
		return binNumber;
	}
	
	public static String rev(String toRev)
	{
		String toRtn = "";
		for (int i = toRev.length() - 1; i >= 0; i--)
		{
			toRtn = toRtn + toRev.charAt(i);
		}
		return toRtn;
	}
	
	public String toString()
	{
		return this.ID + "";
	}
}

