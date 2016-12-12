/******************************************************************************
Class: TSingleton
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk>
Description: 
Quickly turns a class into a singleton by extending class. 
This makes a single globally accessable instance of the given class that can be accessed
anywhere in the program via calling <MyClass>::Instance(). 

This type of coding style can be seen already in the Window class from graphics for games where the 
single window instance can be accessed anywhere in the program via calling Window::GetWindow().

If your interested in learning more about the singleton pattern or good programming patterns,
this wikibook has all you'll ever need! =]
https://en.wikibooks.org/wiki/C%2B%2B_Programming/Code/Design_Patterns


	     (\_/)								
	     ( '_')								
	 /""""""""""""\=========     -----D		
	/"""""""""""""""""""""""\				
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <stddef.h>
#include <mutex>

template <class T>
class TSingleton
{
public:
	//Provide global access to the only instance of this class
	static T* Instance()
	{
		//This if statement prevents the costly thread Lock-step being required each time the instance is requested
		if (!m_pInstance)	
		{
			//Lock /is/ required here though, to prevent multiple threads initialising multiple instances
			// of the class when it turns out it has not been initialised yet
			std::lock_guard<std::mutex> lock(m_Constructed);

			//Check to see if a previous thread has already initialised an instance in the time it took
			// to acquire a lock.
			if (!m_pInstance) 
			{
				m_pInstance = new T();
			}
		}
		return m_pInstance;
	}

	//Provide global access to release/delete this class
	static void Release()
	{
		//Technically this could have another enclosing if statement, but speed is much less of a problem as
		// this should only be called once in the entire program.
		std::lock_guard<std::mutex> lock(m_Constructed);
		if (m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}



protected:
	//Only allow the class to be created and destroyed by itself
	TSingleton() {}
	virtual ~TSingleton() {}


private:
	//Prevent the class from being copied either by '=' operator or by copy constructor
	TSingleton(TSingleton const&)				{}
	TSingleton& operator=(TSingleton const&)	{}

	//Keep a static instance pointer to refer to as required by the rest of the program
	static std::mutex m_Constructed;
	static T* m_pInstance;
};

//Finally make sure that the instance is initialised to NULL at the start of the program
template <class T> std::mutex TSingleton<T>::m_Constructed;
template <class T> T* TSingleton<T>::m_pInstance = NULL;