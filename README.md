# IV-Curve-Tracer
MPPT Solar IV Curve Tracer

Note that the MJH6284G is actually a NPN, but the diagram shows it as a PNP, this is false. 

<img width="1942" height="1036" alt="image" src="https://github.com/user-attachments/assets/98ce0f8c-9941-459f-8848-324a139cd42e" />
<img width="624" height="692" alt="image" src="https://github.com/user-attachments/assets/35a85174-70e8-4ef2-82ee-c31b0bf98c41" />


The IV curve tracer PCB is a crucial piece of hardware meant to characterize the performance of a solar cell module. In Maximum Power Point Tracking applications, the the sensing of the current and voltage allow us to plot a power curve (P=VI) by tracking the cells current and voltage at every stage.
And so the way that this board works is in order to characterize the cell, we must see how it behaves all throughout the spectrum. Including open circuit and short circuit loads, by doing so, we can model the behaviour of each cell accurately. 

Such a board allows us to adjust the electrical load impedance between Solar Panels and a battery to extract the highest possible power at any given point through the Perturb & Observe algorithm. It pretty much just changes the operating voltage and observes if the power output increases or decreases and adjusts in that direction. 

## The Clever Bit
Most commercial designs do use a big programmable electronic load. Our board uses one Darlington transistor as a variable load. As mentioned, in order to test the cells at different voltage ranges, we must sweep the voltage. Because a darling is two transistors stacked, the current gain is huge, this means a tiny control signal from the microcontroller can steer the full cell current. This is a VERY cost effective approach which rely on the firmware to trace the curve point by point. 

To explain how it works, notice every Darlington is common collectors or emitter follows, the left module is meant for high current and the one on the right meant for lower current. When we apply a voltage V_EXT from the MCU, the Emitter of BOTH Q_1 transistors become V_EXT + 1.2 volts. BUT BOTH RB resistors TRY to pull the voltage up to 3.3V, however since BOTH Q1 transistors turn on when the EMITTER is 1.2 VOLTS ABOVE the Base which is VEXT, when it gets to V_EXT + 1.2 V, it dumps the extra current to ground, and so the EMITTERS stay at V_EXT + 1.2V, so we can change V_EXT and it changes that nodes voltage. 

Now since the emitter of both Q1 transistors is V_EXT + 1.2 V, then the base for both Q2 transistors which are NPNs are also that voltage, and thus the emitter follows that voltage - 1.2 V, If you think of it, V_EXT + 1.2 V - 1.2V is SIMPLY V_EXT, AND SO all the VOLTAGE going through the resistors R1 and R2 is SIMPLY V_EXT DIVIDED BY THE RESISTOR VALUE. SINCE R1 is 1 OHM, the current is much higher than R2 which is 10 ohms (10x lower). And so that is how we deal with the varying current by controlling V_EXT.

The neat part is that the PNP’s +1.2 V shift cancels the NPN’s −1.2 V drop, so we don’t need an op-amp or any firmware correction for the offset. So cool!


This was implemented using research here: https://pmc.ncbi.nlm.nih.gov/articles/PMC11279202/
