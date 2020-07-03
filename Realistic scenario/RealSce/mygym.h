/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Technische Universität Berlin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 */


#ifndef MY_GYM_ENTITY_H
#define MY_GYM_ENTITY_H
#include "ns3/stats-module.h"
#include "ns3/opengym-module.h"
#include <vector>
#include <ns3/lte-common.h>
#include <map>

namespace ns3 {

class MyGymEnv : public OpenGymEnv
{
public:
  MyGymEnv ();
  MyGymEnv (double stepTime, uint32_t N1, uint32_t N2, uint16_t N3);
  virtual ~MyGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  Ptr<OpenGymSpace> GetActionSpace();
  Ptr<OpenGymSpace> GetObservationSpace();
  bool GetGameOver();
  Ptr<OpenGymDataContainer> GetObservation();
  float GetReward();
  void calculate_rewards();
  std::string GetExtraInfo();
  bool ExecuteActions(Ptr<OpenGymDataContainer> action);
  static void GetPhyStats(Ptr<MyGymEnv> gymEnv, const PhyTransmissionStatParameters params);
  static void SetInitNumofUEs(Ptr<MyGymEnv> gymEnv, std::vector<uint32_t> num );
  static void GetNumofUEs(Ptr<MyGymEnv> gymEnv, uint16_t CellDec, uint16_t CellInc, uint16_t CellInc_Ues);

private:
  void ScheduleNextStateRead();
  void Start_Collecting();
  static uint8_t Convert2ITB(uint8_t MCSidx);
  static uint8_t GetnRB(uint8_t iTB, uint16_t tbSize);
  void resetObs();
  uint32_t collect;
  double collecting_window = 0.01;
  double block_Thr=0.5;
  uint32_t m_cellCount;
  uint32_t m_userCount;
  uint32_t m_nRBTotal;
  uint8_t m_chooseReward;
  std::vector<uint32_t> m_cellFrequency;
  std::vector<uint32_t> m_UesNum;
  std::vector<float> m_dlThroughputVec;
  float m_dlThroughput;
  std::vector<std::vector<uint32_t>> m_MCSPen;
  std::vector<uint32_t> m_rbUtil;
  std::vector<float> rewards;

  double m_interval = 0.1;
  std::map<uint16_t , std::map<uint16_t, float> > UserThrouput;

};

}

#endif // MY_GYM_ENTITY_H
