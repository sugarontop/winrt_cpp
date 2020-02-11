#pragma once
class DTimer
{
	public :
		DTimer();
		~DTimer();
		void Cancel();
		void CreatePeriodicTimer( std::function<void(DTimer*)> BlinkFunc );
		
	private :
		std::function<void()> BlinkFunc_;
		

		//Windows::System::DispatcherQueue* timer_;
};
