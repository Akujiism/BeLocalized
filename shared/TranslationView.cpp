#include "TranslationView.h"

#include "TranslationStore.h"

#include <Alert.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <Message.h>
#include <ScrollView.h>
#include <SpaceLayoutItem.h>
#include <StringItem.h>
#include <StringView.h>
#include <TextView.h>

#include <stdio.h>

const int32 kMsgSuggest = 'Sgst';
const int32 kMsgSetTranslation = 'Trns';
const int32 kMsgSelectUnit = 'SelU';

class UnitItem : public BStringItem {
public:
			 UnitItem(const char *text, int32 index)
				:
				BStringItem(text),
				mUnitIndex(index) {}
	int32	 UnitIndex() { return mUnitIndex; }
private:
	int32	 mUnitIndex;
};

TranslationView::TranslationView()
	:
	BView("translation view", 0),
	mHideTranslated(false)
{
	mWordsView = new BListView("words view");
	mWordsScrollView = new BScrollView("words scroller", mWordsView, 0, false, true);

	mSourceLabel = new BStringView("source label", "Source:");
	mSource = new BTextView("source");
	mSourceScroll = new BScrollView("source scroller", mSource, 0, false, true);
	mSource->MakeEditable(false);

	mContext = new BStringView("context", "");
	mContextLabel = new BStringView("context label", "Context: ");

	mDeveloperCommentLabel = new BStringView("developer comment label", "Developer comment:");
	mDeveloperComment = new BTextView("developer comment");
	mDeveloperCommentScroll = new BScrollView("developer comment scroller", mDeveloperComment, 0, false, true);

	mTranslated = new BTextView("translated");
	mTranslatedScroll = new BScrollView("translated scroller", mTranslated, 0, false, true);
	mTranslatedLabel = new BStringView("translated label", "Translated:");

	mSuggest = new BButton("suggest", "Suggest", new BMessage(kMsgSuggest));
	mSetAsTranslation = new BButton("set translation", "Set as translation",
		new BMessage(kMsgSetTranslation));

	mButtonsLayout = new BGroupLayout(B_HORIZONTAL);

	BSplitView *v = 
		BLayoutBuilder::Split<>(B_VERTICAL)
			.SetInsets(B_USE_WINDOW_INSETS)
			.Add(mWordsScrollView)
			.AddGroup(B_HORIZONTAL)
				.AddGroup(B_VERTICAL)
					.AddGroup(B_HORIZONTAL)
						.Add(mSourceLabel)
						.AddGlue()
					.End()
					.Add(mSourceScroll)
					.AddGroup(B_HORIZONTAL)
						.Add(mDeveloperCommentLabel)
						.AddGlue()
					.End()
					.Add(mDeveloperCommentScroll)
					.AddGroup(B_HORIZONTAL)
						.Add(mContextLabel)
						.Add(mContext)
						.AddGlue()
					.End()
				.End()
				.AddGroup(B_VERTICAL)
					.AddGroup(B_VERTICAL)
						.AddGroup(B_HORIZONTAL)
							.Add(mTranslatedLabel)
							.AddGlue()
						.End()
						.Add(mTranslatedScroll)
					.End()
					.AddGroup(mButtonsLayout)
					.End()
				.End()
			.End()
		.View();
	
	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.Add(v);

	mWordsView->SetSelectionMessage(new BMessage(kMsgSelectUnit));
	mContextLabel->SetFont(be_bold_font);
	mDeveloperCommentLabel->SetFont(be_bold_font);
	mSourceLabel->SetFont(be_bold_font);
	mTranslatedLabel->SetFont(be_bold_font);
}


void
TranslationView::HideTranslated(bool set)
{
	mHideTranslated = set;
	UnitItem *u = (UnitItem *)mWordsView->ItemAt(mWordsView->CurrentSelection());

	mWordsView->RemoveItems(0, mWordsView->CountItems());
	int32 currentSelection = -1;

	if (u)
		currentSelection = u->UnitIndex() - 1;

	int32 selection = 0;

	for (int32 i = 0; i < mStore->LoadedUnits(); i++) {
		if (!set || mStore->UnitAt(i)->Translated().Length() == 0) {
			mWordsView->AddItem(new UnitItem(mStore->UnitAt(i)->Source(), i));
			if (i == currentSelection) {
				selection = mWordsView->CountItems();
			}
		}
	}

	mWordsView->Select(selection);
}

void
TranslationView::SetStore(TranslationStore *s)
{
	mStore = s;

	while (mButtonsLayout->CountItems() > 0)
		mButtonsLayout->RemoveItem((int32)0);

	mButtonsLayout->AddItem(BSpaceLayoutItem::CreateGlue());

	if (s->CanSetAsTranslation()) {
		mButtonsLayout->AddView(mSetAsTranslation);
		mSetAsTranslation->MakeDefault(true);
	}
	
	if (s->CanSuggest()) {
		mButtonsLayout->AddView(mSuggest);
		mSuggest->MakeDefault(true);
	}
	
	mReceivedUnits = 0;
}

void
TranslationView::AttachedToWindow()
{
	mWordsView->SetTarget(this);
	mSuggest->SetTarget(this);
	mSetAsTranslation->SetTarget(this);
}

void
TranslationView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
	case kMsgGotUnit: {
		TranslationUnit *u;
		if (msg->FindPointer("unit", (void **)&u) != B_OK) {
			break;
		}

		if (!mHideTranslated || u->Translated().Length() == 0) {
			mWordsView->AddItem(new UnitItem(u->Source(), mReceivedUnits));
			if (mWordsView->CurrentSelection() == -1)
				mWordsView->Select(0);
		}
		
		mReceivedUnits++;
		break;
	}
	case kMsgSelectUnit: {
		int index = ((UnitItem *)mWordsView->ItemAt(msg->GetInt32("index", 0)))->UnitIndex();
		mUnit = mStore->UnitAt(index);
		mSource->SetText(mUnit->Source());
		mContext->SetText(mUnit->Context());
		mDeveloperComment->SetText(mUnit->DeveloperComment());
		mTranslated->SetText(mUnit->Translated());
		mWordsView->ScrollToSelection();
		break;
	}
	case kMsgSetTranslation: {
		mUnit->SetTranslated(mTranslated->Text());
		if (!mUnit->SetAsTranslation()) {
			BAlert *a = new BAlert("Failed to set translation", "Failed to set the translation.\nDo you have the right permissions?", "Close");
			a->Go();
			delete a;
		} else {
			mWordsView->Select(mWordsView->CurrentSelection() + 1);
		}
		break;
	}
	case kMsgSuggest: {
		mUnit->SetTranslated(mTranslated->Text());
		if (!mUnit->Suggest()) {
			BAlert *a = new BAlert("Failed to suggest translation", "Failed to suggest the translation.\nDo you have the right permissions?", "Close");
			a->Go();
			delete a;
		} else {
			mWordsView->Select(mWordsView->CurrentSelection() + 1);
		}
		break;
	}
	default:
		BView::MessageReceived(msg);
	}
}