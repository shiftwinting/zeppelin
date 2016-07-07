#include "zp_heartbeat_thread.h"

#include <glog/logging.h>
#include "zp_heartbeat_conn.h"
#include "zp_meta_server.h"

#include "slash_mutex.h"

extern ZPMetaServer* zp_meta_server;

ZPHeartbeatThread::ZPHeartbeatThread(int port, int cron_interval) :
  HolyThread::HolyThread(port, cron_interval) {
  InitServerControlCmdTable(&cmds_);
}

ZPHeartbeatThread::~ZPHeartbeatThread() {
  LOG(INFO) << "ZPHeartbeat thread " << thread_id() << " exit!!!";
}

void ZPHeartbeatThread::CronHandle() {
  //	find out timeout slave and kill them 
  struct timeval now;
  gettimeofday(&now, NULL);
  {
    slash::RWLock l(&rwlock_, true); // Use WriteLock to iterate the conns_
    std::map<int, void*>::iterator iter = conns_.begin();
    while (iter != conns_.end()) {
      DLOG(INFO) << "Slave:  now.tv_sec:" << now.tv_sec << ", last_inter:" << static_cast<ZPHeartbeatConn*>(iter->second)->last_interaction().tv_sec;
      if (now.tv_sec - static_cast<ZPHeartbeatConn*>(iter->second)->last_interaction().tv_sec > 20) {
        LOG(INFO) << "Find Timeout Slave: " << static_cast<ZPHeartbeatConn*>(iter->second)->ip_port();
        close(iter->first);
        // erase item in slaves_
        // TODO
        //zp_meta_server->DeleteSlave(iter->first);

        delete(static_cast<ZPHeartbeatConn*>(iter->second));
        iter = conns_.erase(iter);
        continue;
      }
      iter++;
    }
  }

  // erase it in slaves_;
  {
    slash::MutexLock l(&zp_meta_server->slave_mutex_);
    std::vector<SlaveItem>::iterator iter = zp_meta_server->slaves_.begin();
    while (iter != zp_meta_server->slaves_.end()) {
      DLOG(INFO) << " ip_port: " << iter->node.ip << " port " << iter->node.port << " sender_tid: " << iter->sender_tid << " hb_fd: " << iter->hb_fd << " sender: " << iter->sender << " create_time: " << iter->create_time.tv_sec;
      if (!FindSlave(iter->hb_fd)) {
   //   if ((iter->stage == SLAVE_ITEM_STAGE_ONE && now.tv_sec - iter->create_time.tv_sec > 30)
   //       || (iter->stage == SLAVE_ITEM_STAGE_TWO && !FindSlave(iter->hb_fd))) {
   //     //pthread_kill(iter->tid);

        // Kill BinlogSender
        LOG(WARNING) << "Erase slave (" << iter->node.ip << ":" << iter->node.port << ") from slaves map of heartbeat thread";
        {
          // TODO
          //zp_meta_server->slave_mutex_.Unlock();
          //zp_meta_server->DeleteSlave(iter->hb_fd);
          //zp_meta_server->slave_mutex_.Lock();
        }
        continue;
      }
      iter++;
    }
  }
}

bool ZPHeartbeatThread::AccessHandle(std::string& ip) {
 // if (ip == "127.0.0.1") {
 //   ip = zp_meta_server->host();
 // }

// TODO
//  slash::MutexLock l(&zp_meta_server->slave_mutex_);
//  for (auto iter = zp_meta_server->slaves_.begin(); iter != zp_meta_server->slaves_.end(); iter++) {
//    if (iter->node == ip) {
//      LOG(INFO) << "HeartbeatThread access connection " << ip;
//      return true;
//    }
//  }
//
//  LOG(WARNING) << "HeartbeatThread deny connection: " << ip;
//  return false;
  return true;
}

bool ZPHeartbeatThread::FindSlave(int fd) {
  slash::RWLock(&rwlock_, false);
  return conns_.find(fd) != conns_.end();
}

