#include "ic_types.h"
#include "InfoCrawler.h"
#include "DbManager.h"

void HostNode::push_back(UrlNode *urlnode, bool only_memory) {

    if(fifoqueue->urlnum > 2000 && !only_memory) {
        pthread_rwlock_rdlock(&rwlock_host);
        if (sqlite_host_id == -1) {
            pthread_rwlock_unlock(&rwlock_host);
            pthread_rwlock_wrlock(&rwlock_host);
            if (sqlite_host_id == -1) {
	            int ret = InfoCrawler::getInstance()->getDbManager()->get_hostid_by_domain(host, sqlite_host_id);
	            if (ret == STATE_SUCCESS) {
	            	if (sqlite_host_id == -1) {
			            int ret = InfoCrawler::getInstance()->getDbManager()->insert_host(host);
		              if (ret == STATE_SUCCESS) {
		                  InfoCrawler::getInstance()->getDbManager()->get_hostid_by_domain(host, sqlite_host_id);
		              }	
	            	}
	            }
            }
            pthread_rwlock_unlock(&rwlock_host);
        } else {
            pthread_rwlock_unlock(&rwlock_host);
        }

        if (sqlite_host_id > 0) {
            char buf[sizeof(UrlNode)];
            int len = urlnode->serialize(buf);
            TaskOtherInfo *taskother = InfoCrawler::getInstance()->getTaskScheduleManager()->getTaskOtherInfo(urlnode->taskid); 
            taskother->deletetaskurl(urlnode->url);

            InfoCrawler::getInstance()->getDbManager()->insert_unfetched_url(urlnode->url, buf, len, sqlite_host_id, true) ;
            delete urlnode;
            return;
        }
    }
    fifoqueue->push_back(urlnode);

}

void HostNode::push_front(UrlNode *urlnode, bool only_memory) {
    if(fifoqueue->urlnum > 2000 && !only_memory) {
	 pthread_rwlock_rdlock(&rwlock_host);
	 if (sqlite_host_id == -1) {
            pthread_rwlock_unlock(&rwlock_host);
            pthread_rwlock_wrlock(&rwlock_host);
            if (sqlite_host_id == -1) {
	            int ret = InfoCrawler::getInstance()->getDbManager()->get_hostid_by_domain(host, sqlite_host_id);
	            if (ret == STATE_SUCCESS) {
	            	if (sqlite_host_id == -1) {
			            int ret = InfoCrawler::getInstance()->getDbManager()->insert_host(host);
		              if (ret == STATE_SUCCESS) {
		                  InfoCrawler::getInstance()->getDbManager()->get_hostid_by_domain(host, sqlite_host_id);
		              }	
	            	}
	            }
            }
            pthread_rwlock_unlock(&rwlock_host);
        } else {
            pthread_rwlock_unlock(&rwlock_host);
        }

        if (sqlite_host_id > 0) {
            char buf[sizeof(UrlNode)];
            int len = urlnode->serialize(buf);
            TaskOtherInfo *taskother = InfoCrawler::getInstance()->getTaskScheduleManager()->getTaskOtherInfo(urlnode->taskid); 
            taskother->deletetaskurl(urlnode->url);
            InfoCrawler::getInstance()->getDbManager()->insert_unfetched_url(urlnode->url, buf, len, sqlite_host_id, false);
            delete urlnode;
            return;
        }
    }
    fifoqueue->push_front(urlnode);
}
