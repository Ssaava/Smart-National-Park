# Smart National Park Tourist Entry, Exit and Fee Collection System

Queen Elizabeth wishes to automate fee collection, tourist entry, exit into the national. All passengers
are expected to enter the park using a car, identified by a number plate. The details of the car occupants
are also taken before the car enters the park. When a vehicle approaches the park entrance gate, it
steps on a bridge, just outside the entrance gate, which is causes the switch under the bridge to be
toggle and trigger an interrupt on the ATMEGA microcontroller to detect the incoming visitors. This
causes the entrance bell (buzzer) to go off and for the message incoming tourist vehicle to be printed
on the LCD screen in order to signal the attendant to take registration of the incoming tourists. The
attendant registers the number of children 10 years and below, the car number plate and tourists above
10 years. After registration, it takes exactly 3 seconds for the car to enter through the gate and 2 second
for the gate to close. The number of tourists categorized under children and adults is incremented
depending on how many were in the car. It is a park policy that the driver enters the park free of
charge. Hence, they don’t pay the entrance fee. At a given point in time, each event that happens is
printed on the LCD screen. For instance, car at gate, gate closing, registration, etc. In case the park is
full to capacity, incoming cars are not allowed in. They instead wait until some tourists leave the park.
The LCD should display park full in case it is.

---

Once inside the park, tourists have access to a self-service fridge and can only pick water, which is at
a rate of UGX 1,500 each. In case one wants a bottle of water, they press the keypad and enter the
number of bottles they want to buy. The machine prints the amount of money the transaction will
cost the tourist. When the tourist accepts, the money slot opens (by showing motors make 3 rotations)
during which the tourist is expected to insert the required funds. It The money slot opens for 2 seconds
allowing the tourist time to insert the required funds. Thereafter, the machine dispenses the required
number of bottles. It takes 2 revolutions of a motor to dispense one bottle. In case there are more
than one bottle to be dispensed, the fridge takes 2 seconds before it can dispense another bottle. The
fridge can be replenished with more water by the attendant but he requires a password to access the
function on the serial console. This password is stored in EEPROM. The attendant can also pick all
the collected money at a given time using the menu on the serial console using an appropriate
command.

---

At the exit gate, the same scenarios that happen during entrance happen. However, the registration of
the outgoing car is done and the number of people in the park is reduced accordingly.
The system is configured with the amount of fee that every child 10 years and below pays and that
those that are above 10 years pay. This configuration is done before the tourists can access the park
and are saved to EEPROM memory, Hence, even if the system is shut down, the configurations are
not lost. The system also should be configured to store the capacity of the tourists who can be in the
park at a given time.

At a given point in time, the attendant may check the following via a serial console menu

- How many tourists, categorized by age group are in the park
- Which vehicles are still in the park?
- How much has been collected by the park aggregated by the fridget money and entrance
  fund
- How many drivers are in the park?
- Number of bottles in the fridge
- Replenish Fridge
- Login
- How many cars are waiting outside the park
- Is park full or not?
- Etc

**Assumption:** At the beginning, there are no tourists inside the park since they all leave at the end of
the day. Any other assumptions to improve the work will bring extra scores to the respective group
Instructions

- Use proteus to simulate the above exercise
- Groups **MUST not EXCEED 5 MEMBERS** and all members should be registered
- Deadline for submission of the assignment is **5th Nov 2023**
- Software Engineering students to upload the assignment on Muele while the IoT students will share the
  work on email at **mnsabagwa@cit.ac.ug** . **No BSSE STUDENT IS ALLOWED TO
  SEND THEIR WORK TO THE EMAIL**
- What to submit
  - the C code with **group member names**,
  - the .hex file and the
  - the schematic
- There will be a presentation on 6th Nov 2023.
- I will listen to the presentations and
  award marks there and then.

## Confirm the groups [from](https://docs.google.com/spreadsheets/d/12tDuvmDEFpV9yaT2xKbajw9heQQAHNAD/edit?usp=sharing&ouid=112023987947581394086&rtpof=true&sd=true)

if you wish to be moved to another group, which has less than 5 members, let me know
