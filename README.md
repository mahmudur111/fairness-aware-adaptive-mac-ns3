# Fairness-Aware Adaptive MAC for Multi-Hop IEEE 802.11 Ad Hoc Networks

This repository contains the ns-3 simulation work for my research project on fairness-aware adaptive MAC mechanisms in multi-hop IEEE 802.11 ad hoc networks.

## Research Objective

The project investigates whether a fairness-aware adaptive MAC mechanism can improve channel-access fairness under different network conditions while maintaining acceptable throughput, delay, packet delivery, and packet-loss performance.

## Current Progress

- ns-3 simulation environment configured
- IEEE 802.11 ad hoc baseline implemented
- Three-node multi-hop topology created
- AODV routing configured
- UDP traffic generation implemented
- FlowMonitor integrated
- PDR, packet loss, delay, and throughput measured
- Simulation results exported to CSV
- NetAnim visualization implemented
- Fairness experiment under development

## Current Baseline Topology

Source (Node 0) -> Relay (Node 1) -> Destination (Node 2)

Node spacing: 50 m  
Maximum communication range: 60 m

## Preliminary Baseline Result

100 UDP packets were transmitted from the source to the destination.

- Packets transmitted: 100
- Packets received: 100
- Packet loss: 0
- PDR: 100%
- Average delay: 0.715347 ms
- Throughput: 85.0056 kbps

## Tools

- ns-3
- C++
- FlowMonitor
- NetAnim
- Python (planned for data analysis and visualization)

## Project Status

Work in progress. The next stage introduces competing traffic flows and evaluates fairness using Jain's Fairness Index.
