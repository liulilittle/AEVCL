#pragma once

#include "stdafx.h"
#include <vector>
#include "IChannelBase.h"
#include "look.h"

using namespace std;

class CChannelService
{
private: 
	vector<IChannelBase*> m_channels;
	CLook* m_look;

public:
	CChannelService();
	~CChannelService();

public:
	int __declspec(property(get = get_Count)) Count;
	IChannelBase* Add(IChannelBase* channel);
	void RemoveAll();
	void Remove(IChannelBase* channel);
	bool Contains(IChannelBase* channel);

private:
	int get_Count();
	int IndexOf(IChannelBase* channel);
};

