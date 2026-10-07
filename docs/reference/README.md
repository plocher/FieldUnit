In traditional Centralized Traffic Control (CTC) systems (originally pioneered by companies like General Railway Signal and Union Switch & Signal), the division of labor and technical components relied on specific, strict signaling terminology. [1, 2] 
The primary division in CTC architecture is between the Office and the Field. Here is how the dispatcher, lineside, in-plant components, and traditional tower operations are structured under this scheme: [3, 4] 
## 1. The Dispatcher (The "Office")
The dispatcher's environment and the logic components located at headquarters are referred to as the Office or Control Station. [5] 

* The Control Machine: The physical desk, levers, buttons, and schematic tracking lights handled by the dispatcher. [1, 4] 
* Office Line Coding Units / Office Application Logic: Non-vital (safety-wise) relays or hardware located at the office that package the dispatcher’s commands into electronic pulses to send downstream. [4] 
* OS Track Circuit (On Sheet): The tracking mechanism on the dispatcher's board. When a train physically occupies a critical junction or turnout, it lights up an "OS Section" on the panel, indicating to the dispatcher that the train is legally "On Sheet" at that specific block. [6, 7] 

## 2. Lineside and In-Plant Components (The "Field")
Everything outside of the central office is collectively referred to as the Field. Within the field, components are split into vital interlocking plants and lineside infrastructure: [3, 4] 

* Control Points (CP): The specific remote field locations—usually a turnout, crossover, or siding end—where the dispatcher is given direct control over switches and signals. [6, 8] 
* The Interlocking Plant ("In-Plant"): The localized system of track switches, switch machines, and signals that are interconnected. Crucially, the "Vital Logic" (the fail-safe safety circuitry that prevents a dispatcher from accidentally setting conflicting routes or throwing a switch underneath a moving train) is entirely in-plant / local. The office merely sends a "request," but the local field interlocking equipment verifies if it is safe to execute. [6, 9] 
* Lineside (Wayside) Infrastructure:
* Home Signals (Interlocking Signals): The absolute signals guarding the entry into a Control Point interlocking limits. They display a "Stop" indication as their most restrictive aspect and cannot be passed without dispatcher authority.
   * Intermediate Signals: Automatic signals positioned between Control Points. The dispatcher does not control these; they operate autonomously based on track occupancy (via track circuits) to maintain safe braking distances between following trains.
   * The Code Line: The lineside infrastructure—historically a physical pair of copper wires running along telephone poles parallel to the tracks—that carried the code pulses between the Office and the Field. [4, 6, 10] 

------------------------------
## How Tower-Based Operations Were Included
Before CTC, railroads relied on decentralized Interlocking Towers spaced every few miles. Local tower operators manually manipulated heavy mechanical levers to control local switches and signals based on written "train orders" issued by a central dispatcher. [2, 9, 11] 
When CTC territory was established, towers were integrated or phased out using three distinct methods:

| Integration Method | How it Worked |
|---|---|
| Complete Elimination (Remote Control) | The vast majority of local towers were entirely closed. The local mechanical levers were replaced by electronic switch machines and remote relays. Control of the plant was handed directly to the central dispatcher, turning the former tower's jurisdiction into an unmanned Control Point (CP). |
| Control Operator Status | In highly complex areas—such as major passenger terminals or busy rail junctions—a physical tower was kept open. However, the local tower worker was designated as a Control Operator. They retained localized control over their layout but were legally subordinate to the central CTC dispatcher, needing permission before routing trains out of the tower limits into the dispatcher's main line CTC territory. |
| The "Traffic Lever" Hybrid System | Where CTC met an active, independent tower territory, a system of Traffic Levers (or "unlocks") was used. For a train to flow from the dispatcher's CTC track into the tower's manual interlocking, both the central dispatcher and the local tower operator had to manipulate levers simultaneously. This established a unified "flow of traffic," mechanically or electrically locking out opposing signals on both ends to prevent a head-on collision. |


[1] [https://www.youtube.com](https://www.youtube.com/watch?v=BPLg5Bdmvrg)
[2] [https://www.scribd.com](https://www.scribd.com/document/275113831/Ctc)
[3] [https://tracsis-us.com](https://tracsis-us.com/solutions/computer-aided-dispatching/centralized-traffic-control)
[4] [https://ctcparts.com](http://ctcparts.com/?page_id=228)
[5] [https://en.wikipedia.org](https://en.wikipedia.org/wiki/Centralized_traffic_control)
[6] [https://ctcparts.com](http://ctcparts.com/?page_id=228)
[7] [https://ctcparts.com](http://ctcparts.com/?page_id=322)
[8] [https://stationinnpa.com](https://stationinnpa.com/train-terms-to-know/)
[9] [https://en.wikipedia.org](https://en.wikipedia.org/wiki/Centralized_traffic_control)
[10] [https://www.logicrailtech.com](https://www.logicrailtech.com/ctcdemo.htm)
[11] [https://www.trains.com](https://www.trains.com/mrr/how-to/prototype-railroads/why-do-railroads-use-towers/)
