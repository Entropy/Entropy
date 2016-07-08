#pragma once

namespace entropy
{
	namespace util
	{
		// Based on http://www.yolinux.com/TUTORIALS/C++Singleton.html
		template <class T>
		class Singleton
		{
		public:
			//--------------------------------------------------------------
			static T * X()
			{
				return (Singleton::Instance ? Singleton::Instance : Singleton::Instance = new T());
			}

			//--------------------------------------------------------------
			static void Destroy()
			{
				if (Singleton::Instance)
				{
					delete Singleton::Instance;
					Singleton::Instance = nullptr;
				}
			}

		private:
			Singleton() = delete;
			Singleton(const Singleton &) = delete;				// Copy construct
			Singleton(Singleton &&) = delete;					// Move construct
			Singleton & operator=(const Singleton &) = delete;  // Copy assign
			Singleton & operator=(Singleton &&) = delete;		// Move assign

			static T * Instance;
		};

		//--------------------------------------------------------------
		template <class T>
		T * Singleton<T>::Instance = nullptr;
	}
}
