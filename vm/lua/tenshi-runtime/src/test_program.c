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

#include <stdio.h>
#include <string.h>

#include "inc/mboxlib.h"
#include "inc/runtime_entry.h"

int main(int argc, char **argv) {
  printf("Hello world!\n");

  TenshiRuntimeState s = TenshiRuntimeInit();

  printf("Allocated state: %p\n", s);

  // const char studentcode[] =
  //   "x = 42";

  // const char studentcode[] =
  //   "i = 0\n"
  //   "while true do\n"
  //   "    print(\"Hello world!\")\n"
  //   "    print(math.pi)\n"
  //   "    i = i + 1\n"
  //   "    if i > 300 then\n"
  //   "        get_own_actor():stop()\n"
  //   "    end\n"
  //   "end";

  // const char studentcode[] =
  //   "function b()\n"
  //   "    for i = 1,500,1 do\n"
  //   "        send(a_actor, i, a_actor, i, a_actor, i)\n"
  //   "        print(\"sent: \" .. i)\n"
  //   "    end\n"
  //   "end\n"
  //   "\n"
  //   "a_actor = get_own_actor()\n"
  //   "start_actor(b)\n"
  //   "for i = 1,500,1 do\n"
  //   "    print(\"recv: \" .. recv(a_actor))\n"
  //   "end";

  const char studentcode[] =
    "install_trap_global()\n"   // Normally this would be further down
    "input_dev = get_device('input')\n"
    "input = triggers.changed(input_dev)\n"
    "output = get_device('output')\n"
    "\n"
    "print('units.mega = ' .. units.mega)\n"
    "print('units.kilo = ' .. units.kilo)\n"
    "print('units.mili = ' .. units.mili)\n"
    "print('units.micro = ' .. units.micro)\n"
    "print('units.nano = ' .. units.nano)\n"
    "print('units.inch = ' .. units.inch)\n"
    "print('units.pound = ' .. units.pound)\n"
    "print('units.deg = ' .. units.deg)\n"
    "\n"
    "while true do\n"
    "    print('about to recv')\n"
    "    local x = input:recv()\n"
    "    if x == nil then x = 0 end\n"
    "    print('recv: ' .. x)\n"
    "    x = x + 1"
    "    print('sending using value')\n"
    "    output.value = x\n"
    "    print('sent: ' .. x)\n"
    "end";

  TenshiActorState a;

  int ret = MBoxCreateActuator(s, "output", 6);
  printf("MBoxCreateActuator: %d\n", ret);

  ret = MBoxCreateSensor(s, "input", 5);
  printf("MBoxCreateSensor: %d\n", ret);

  ret = LoadStudentcode(s, studentcode, strlen(studentcode), &a);
  printf("LoadStudentcode: %d, TenshiActorState: %p\n", ret, a);

  ret = ActorSetRunnable(a, 1);
  printf("ActorSetRunnable: %d\n", ret);

  int i = 0;

  while (i < 100) {
    printf("-----> Sent into sensor: %d\n", i / 2);
    TenshiMainStackPushInt(s, i / 2);
    ret = MBoxSendSensor(s, "input", 5);
    printf("MBoxSendSensor: %d\n", ret);
    ret = TenshiRunQuanta(s);
    printf("Ran quanta %d, ret = %d\n", i, ret);

    update_info *ui_orig = MBoxGetActuatorsChanged(s);
    printf("MBoxGetActuatorsChanged: %p\n", ui_orig);
    update_info *ui = ui_orig;
    while (ui) {
      printf("Actuator set: %s (%d values)\n", ui->id, ui->num_data);
      for (int j = 0; j < ui->num_data; j++) {
        ret = MBoxRecvActuator(s, ui->id, ui->id_len);
        printf("MBoxRecvActuator: %d\n", ret);
        int x = TenshiMainStackGetInt(s);
        printf("<----- Got data out: %d\n", x);
      }
      ui = ui->next;
    }
    MBoxFreeUpdateInfo(ui_orig);

    i++;
  }

  TenshiRuntimeDeinit(s);

  printf("Done!\n");

  return 0;
}
