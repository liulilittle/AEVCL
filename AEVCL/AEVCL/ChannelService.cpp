#include "stdafx.h"
#include "ChannelService.h"

CChannelService::CChannelService()
{
	m_look = new CLook;
}

CChannelService::~CChannelService()
{
	if (m_look != nullptr)
		m_look->~CLook();
	m_look = nullptr;
}

IChannelBase* CChannelService::Add(IChannelBase* channel)
{
	m_look->Look();
	__try {
		if (channel == nullptr) 
			throw exception("通道不可是空的");
		if (!this->Contains(channel)) 
			m_channels.push_back(channel);
		return channel;
	}
	__finally {
		m_look->Unlook();
	}
}

int CChannelService::get_Count()
{
	return m_channels.size();
}

int CChannelService::IndexOf(IChannelBase* channel)
{
	auto element = find(m_channels.begin(), m_channels.end(), channel);
	if (element == m_channels.end())
		return -1;
	return m_channels.end() - element;
}

void CChannelService::RemoveAll()
{
	m_look->Look();
	m_channels.clear();
	m_look->Unlook();
}

void CChannelService::Remove(IChannelBase* channel)
{
	if (channel != nullptr) {
		int offset = this->IndexOf(channel);
		if (offset < 0) {
			m_look->Look();
			m_channels.erase(m_channels.begin() + offset);
			m_look->Unlook();
		}
	}
}

bool CChannelService::Contains(IChannelBase* channel)
{
	int offset = this->IndexOf(channel);
	return offset > -1;
}
