# Week 3 Documentation

Planning documents and testing notes from building and evaluating the UDP proxy.

---

## Files

### Planning (`week3_plan.md`)

The initial plan for Week 3, broken down into stages:

**Stage 1:** Simple pass-through proxy (transparent routing)
**Stage 2:** Add delay queue for latency simulation  
**Stage 3:** Add packet loss simulation
**Stage 4:** Test with treasure hunt game
**Stage 5:** Polish and command-line configuration

This document was written before starting implementation. Comparing the plan to what actually got built reveals what worked (stages 1-3) and what changed (stages 4-5 are ongoing).

Worth reading to see how I approached planning a multi-day project and breaking it into manageable pieces.

---

### Testing Notes (`latency_notes.md`)

Raw field notes from stress-testing the treasure hunt game through the proxy. Documents what broke under different network conditions.

**Key findings:**
- 50ms latency made movement feel sluggish
- 100ms+ became nearly unplayable without client prediction
- Packet loss completely broke game state synchronization
- Critical events (connections, treasure spawns) need reliability

These notes directly informed what needs to be fixed in future weeks—specifically the need for client prediction, entity interpolation, and reliable event delivery.

**Read this to understand:**
- Why network testing matters
- How real-world conditions expose architectural flaws
- What problems client prediction solves
- The difference between "works on localhost" and "works on the internet"

---

## Context

Week 3's goal was understanding how the treasure hunt game behaves under realistic network conditions. These documents capture both the planning phase and the empirical results.

The planning document shows intentional design—breaking a complex project into stages and tackling them sequentially.

The testing notes show discovery—finding out empirically what breaks and why. Some findings were expected (latency makes things sluggish), some were surprising (packet loss completely breaking treasure state).

Together, they tell the story of Week 3: build a tool (proxy), use it to test (treasure hunt), discover problems (latency + packet loss = broken), which informs future work (client prediction, interpolation).

---

**These documents are the "why" behind Week 3's code.**
