#include "BeLocalizedApp.h"

#include "LogInWindow.h"
#include "PootleAPI/Pootle.h"

#include <stdio.h>

void
BeLocalizedApp::ArgvReceived(int32 argc, char **argv)
{
}


void
BeLocalizedApp::MessageReceived(BMessage *msgrecv)
{
	msgrecv->PrintToStream();
	switch (msgrecv->what) {
	case kMsgPootleInited: {
		msgrecv->FindPointer("pootle", (void **)&mPootle);
		BObjectList<PootleProject> msg = mPootle->Projects()->Get();
		for (int32 i = 0; i < msg.CountItems(); i++) {
			PootleProject *pr = msg.ItemAt(i);
			printf("Project %d: %s (from %s)\n", i, pr->FullName().String(), pr->SourceLanguage().FullName().String());\
		}
		int index;
		printf("Choose project: ");
		scanf("%d", &index);
		PootleProject *pr = msg.ItemAt(index);
		for (int32 i = 0; i < pr->CountTranslationProjects(); i++) {
			PootleTranslationProject p = pr->GetTranslationProject(i);
			printf("%4d: %s\n", i, p.Language().FullName().String());
		}
		printf("Choose translation project: ");
		scanf("%d", &index);

		PootleTranslationProject tp = pr->GetTranslationProject(index);
		for (int32 i = 0; i < tp.CountStores(); i++) {
			PootleStore st = tp.GetStore(i);
			printf("%4d: %s\n", i, st.File().String());
		}

		printf("Choose store: ");
		scanf("%d", &index);
		PootleStore st = tp.GetStore(index);
		
		for (int32 i = 0; i < st.CountUnits(); i++) {
			PootleUnit u = st.GetUnit(i);
			printf("%4d: %s\n", i, u.Source().text.String());
		}
		
		printf("Choose unit: ");
		scanf("%d", &index);
		PootleUnit u = st.GetUnit(index);
		printf("Source: %s\n", u.Source().text.String());
		printf("(comment): %s\n", u.TranslatorComment().String());
		printf("Target: %s\n\n", u.Target().text.String());
		char buffer[256];
		PootleSuggestion s;
		fgets(buffer, 256, stdin);

		printf("Suggest: ");
		fgets(buffer, 256, stdin);
		s.SetTarget(buffer);

		printf("(translated comment): ");
		fgets(buffer, 256, stdin);
		s.SetTranslatorComment(buffer);

		s.SetUnit(u);
		s.Create(mPootle->Suggestions());
		Quit();
		break;
	}
	default:
		BApplication::MessageReceived(msgrecv);
		break;
	}
}


void
BeLocalizedApp::ReadyToRun()
{
	LogInWindow *w = new LogInWindow(BRect(0, 0, 100, 100));
	w->CenterOnScreen();
	w->Show();
}
