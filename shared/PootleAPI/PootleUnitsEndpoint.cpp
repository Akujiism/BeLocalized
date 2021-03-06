#include "PootleUnitsEndpoint.h"

#include "Pootle.h"
#include "PootleStoresEndpoint.h"

#include <assert.h>
#include <stdio.h>


int
PootleUnit::CountSuggestions()
{
	_EnsureData();
	BMessage suggestions;
	mData.FindMessage("suggestions", &suggestions);
	return suggestions.CountNames(B_ANY_TYPE);
}


PootleSuggestion
PootleUnit::GetSuggestion(int index)
{
	char buffer[33];
	sprintf(buffer, "%d", index);

	_EnsureData();
	BMessage suggestions;
	mData.FindMessage("suggestions", &suggestions);
	BString str = suggestions.GetString(buffer, "");

	return mEndpoint->API()->Suggestions()->GetByUrl(str);
}


BObjectList<PootleSuggestion>
PootleUnit::Suggestions()
{
	BMessage suggestions;
	mData.FindMessage("suggestions", &suggestions);

	BObjectList<BString> urls(20, true);
	
	int32 count = suggestions.CountNames(B_ANY_TYPE);

	char buffer[33];
	for (int32 i = 0; i < count; i++) {
		sprintf(buffer, "%d", i);
		urls.AddItem(new BString(suggestions.GetString(buffer, "")));
	}
	
	return mEndpoint->API()->Suggestions()->GetByList(urls);

}


PootleUser
PootleUnit::CommentedBy()
{
	_EnsureData();
	return mEndpoint->API()->Users()->GetByUrl(
		mData.GetString("commented_by", ""));
}


BString
PootleUnit::CommentedOn()
{
	_EnsureData();
	return BString(mData.GetString("commented_on", ""));
}


BString
PootleUnit::Context()
{
	_EnsureData();
	return BString(mData.GetString("context", ""));
}


BString
PootleUnit::DeveloperComment()
{
	_EnsureData();
	return BString(mData.GetString("developer_comment", ""));
}


BString
PootleUnit::Locations()
{
	_EnsureData();
	return BString(mData.GetString("locations", ""));
}


BString
PootleUnit::MTime()
{
	_EnsureData();
	return BString(mData.GetString("mtime", ""));
}


BString
PootleUnit::SubmittedOn()
{
	_EnsureData();
	return BString(mData.GetString("submitted_on", ""));
}


PootleUser
PootleUnit::SubmittedBy()
{
	_EnsureData();
	return mEndpoint->API()->Users()->GetByUrl(
		mData.GetString("submitted_by", ""));
}


BString
PootleUnit::TranslatorComment()
{
	_EnsureData();
	return BString(mData.GetString("translator_comment", ""));
}


Translation
PootleUnit::Source()
{
	_EnsureData();
	Translation trans;

	trans.text = mData.GetString("source_f", "");
	trans.length = (int)mData.GetDouble("source_length", 0.0);
	trans.wordcount = (int)mData.GetDouble("source_wordcount", 0.0);
	
	return trans;
}


Translation
PootleUnit::Target()
{
	_EnsureData();
	Translation trans;

	trans.text = mData.GetString("target_f", "");
	trans.length = (int)mData.GetDouble("target_length", 0.0);
	trans.wordcount = (int)mData.GetDouble("target_wordcount", 0.0);
	
	return trans;
}


int
PootleUnit::State()
{
	_EnsureData();
	
	return (int)mData.GetDouble("state", 0.0);
}


PootleStore
PootleUnit::Store()
{
	_EnsureData();
	return mEndpoint->API()->Stores()->GetByUrl(
		mData.GetString("store", ""));
}


void
PootleUnit::_EnsureData()
{
	if(!mData.IsEmpty())
		return;

	mData = mEndpoint->_SendRequest("GET", mUri);
	mEndpoint->_add_to_cache(mUri, *this);
}

PootleUnit::PootleUnit(
	_Endpoint *endpoint, BMessage &data)
	:
	mEndpoint(endpoint),
	mData(data),
	mUri(data.GetString("resource_uri", ""))
{
	mEndpoint->_add_to_cache(mUri, *this);
}


PootleUnit::PootleUnit(
	_Endpoint *endpoint, int id)
	:
	mEndpoint(endpoint)
{
	mUri.SetToFormat("%d/", id);
	if (mEndpoint->_cache_contains(id))
		*this = mEndpoint->_get_from_cache(id);
}


PootleUnit::PootleUnit(
	_Endpoint *endpoint, BString uri)
	:
	mEndpoint(endpoint),
	mUri(uri)
{
	if (mEndpoint->_cache_contains(uri))
		*this = mEndpoint->_get_from_cache(uri);
}
