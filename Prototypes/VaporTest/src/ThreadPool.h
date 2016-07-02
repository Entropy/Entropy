#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <vector>
#include <functional>
#include "ofThreadChannel.h"

class ThreadPool{
		ThreadPool()
		:threads(std::thread::hardware_concurrency()){
			for(auto & thread: threads){
				thread = std::thread([&]{
					std::function<void()> func;
					while(channel.receive(func)){
						func();
					}
				});
			}
		}
	public:
		static ThreadPool & pool(){
			static ThreadPool * pool = new ThreadPool();
			return *pool;

		}

		void addTask(std::function<void()> func){
			channel.send(func);
		}


	private:
		std::vector<std::thread> threads;
		ofThreadChannel<std::function<void()>> channel;
};

#endif // THREADPOOL_H
