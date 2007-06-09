/**
 * This file is part of the System Settings package
 * Copyright (C) 2005 Benjamin C Meyer (ben+systempreferences at meyerhome dot net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "mainwindow.h"

#include <QObject>
#include <QAction>
#include <QIcon>
#include <kstandardaction.h>
#include <ktoolbarlabelaction.h>
#include <ktoggletoolbaraction.h>
#include <kaboutapplicationdialog.h>
#include <q3whatsthis.h>
#include <QLabel>
#include <q3vbox.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <kaction.h>
#include <qtoolbutton.h>
#include <klocale.h>
#include <kservicegroup.h>
#include <qlayout.h>
#include <q3widgetstack.h>
#include <qtimer.h>
#include <kiconloader.h>
#include <kcmoduleloader.h>
#include <kpagedialog.h>
/*#include <k3iconviewsearchline.h>*/
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kcmoduleproxy.h>
#include <kbugreport.h>
#include <kmenubar.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <QToolButton>
#include <qtabbar.h>

#include "kcmsearch.h"
#include "modulesview.h"
#include "moduleiconitem.h"
#include "kcmodulemenu.h"
#include "kcmultiwidget.h"

MainWindow::MainWindow(bool embed, const QString & menuFile, QWidget *parent) :
				KXmlGuiWindow(parent), menu(NULL), embeddedWindows(embed),
				groupWidget(NULL), selectedPage(0), dummyAbout(NULL) {

	// Load the menu structure in from disk.
	menu = new KCModuleMenu( menuFile );

	moduleTabs = new KTabWidget(this, QTabWidget::Top|QTabWidget::Rounded);
	buildMainWidget();
	buildActions();
	setupGUI(ToolBar|Save|Create,QString::null);
	widgetChange();
}

MainWindow::~MainWindow()
{
	delete moduleTabs;
	delete windowStack;
	delete menu;	
	delete dummyAbout;
}

void MainWindow::buildMainWidget()
{
	windowStack = new Q3WidgetStack( this, "widgetstack" );

	// Top level pages.
	Q3ValueList<MenuItem> subMenus = menu->menuList();
	Q3ValueList<MenuItem>::iterator it;
	KCScrollView *modulesScroller;
	moduleTabs->show();
	for ( it = subMenus.begin(); it != subMenus.end(); ++it ) {
		if( (*it).menu ) {
			modulesScroller = new KCScrollView(moduleTabs);
			ModulesView *modulesView = new ModulesView( menu, (*it).subMenu, modulesScroller->viewport(), "modulesView" );
			modulesViewList.append(modulesView);
			connect(modulesView, SIGNAL(itemSelected(Q3IconViewItem* )), this, SLOT(slotItemSelected(Q3IconViewItem*)));
			modulesScroller->addChild(modulesView);
			moduleTabs->addTab(modulesScroller, (*it).caption);
			overviewPages.append(modulesScroller);
		}
	}

	windowStack->addWidget(moduleTabs);
	windowStack->raiseWidget(moduleTabs);
	setCentralWidget(windowStack);
}

void MainWindow::buildActions()
{
  //	KStandardAction::quit(this, SLOT( close() ), qobject_cast<QObject*>(actionCollection()));
  actionCollection()->addAction(KStandardAction::Quit, qobject_cast<QObject*>(this), SLOT(close()));

// 	resetModule = new KAction(i18n("Undo Changes"), 0, qobject_cast<QObject*>(this),
// 								SLOT(showAllModules()), actionCollection(), "resetModule" );
// 	resetModule->setEnabled(false);
	resetModule = actionCollection() -> addAction("resetModule");
  resetModule->setText(i18n("Undo Changes"));
  connect(resetModule, SIGNAL(triggered()),
          this, SLOT(close()));
	resetModule->setEnabled(false);

// 	defaultModule = new KAction(i18n("Reset to Defaults"), 0, qobject_cast<QObject*>(this),
// 								SLOT(showAllModules()), actionCollection(), "defaultModule" );
// 	defaultModule->setEnabled(false);
  defaultModule = actionCollection() -> addAction("defaultModule");
  defaultModule->setText(i18n("Reset to Defaults"));
  connect(defaultModule, SIGNAL(triggered()),
          this, SLOT(showAllModules()));;
  defaultModule->setEnabled(false);

	if( embeddedWindows ) {
// 		showAllAction = new KAction(i18n("Overview"), QApplication::reverseLayout() ? "forward" : "back", 0,
//                                 qobject_cast<QObject*>(this), SLOT(showAllModules()), actionCollection(),
//                                 "showAll" );
// 		showAllAction->setEnabled(false);
    showAllAction = actionCollection() -> addAction("showAll");
    showAllAction->setText(i18n("Overview"));
    connect(showAllAction, SIGNAL(triggered()),
            this, SLOT(showAllModules()));
    showAllAction->setEnabled(false);
	}

// 	aboutModuleAction = new KAction(i18n("About Current Module"), 0, qobject_cast<QWidget*>(this),
//                                   SLOT(aboutCurrentModule()), actionCollection(), "help_about_module");
  aboutModuleAction = actionCollection() -> addAction("help_about_module");
  aboutModuleAction->setText(i18n("About Current Module"));
  connect(aboutModuleAction, SIGNAL(triggered()),
          this, SLOT(aboutCurrentModule()));


	resetModuleHelp();

	// Search
	Q3HBox *hbox = new Q3HBox(0);
	hbox->setMaximumWidth( 400 );

	/*FIXME*/
	KcmSearch* search = new KcmSearch(&modulesViewList, hbox, "search");
	hbox->setStretchFactor(search,1);
	connect(search, SIGNAL(searchHits(const QString &, int *, int)), this, SLOT(slotSearchHits(const QString &, int *, int)));

	Q3VBox *vbox = new Q3VBox(hbox);
	generalHitLabel = new QLabel(vbox);
	vbox->setStretchFactor(generalHitLabel,1);
	advancedHitLabel = new QLabel(vbox);
	vbox->setStretchFactor(advancedHitLabel,1);

	hbox->setStretchFactor(vbox,1);

	// "Search:" label	
	QLabel *searchLabel = new QLabel( this, "SearchLabel");
	searchLabel->setText( i18n("&Search:") );
	searchLabel->setFont(KGlobalSettings::toolBarFont());
	searchLabel->setMargin(2);
  //******* STOPPED **********/

// KWidgetAction* action = new KWidgetAction( findCombo, i18n("Find Combo"),
//                                             Qt::Key_F6, this, SLOT( slotFocus() ),
//                                             actionCollection(), "find_combo");

//  KAction *action = new KToolBarLabelAction( action, i18n( "Find "), "find_label" );
//  action->setShortcut( Qt::Key_F6 );
//  connect( action, SIGNAL( triggered() ), this, SLOT( slotFocus() ) );

  searchText = new KToolBarLabelAction(i18n("&Search:"), searchLabel);
  searchText->setShortcut(Qt::Key_F6);
//   connect(action, SIGNAL(triggered()),
//           searchLabel, SLOT( 
// // 	searchText = new KWidgetAction( searchLabel, i18n("&Search:"), Qt::Key_F6, 0, 0, actionCollection(), "searchText" );
// // 	searchLabel->setBuddy( search );

	// The search box.
  searchAction = new KToolBarLabelAction(i18n("Search System Settings"), hbox);
// // 	searchAction = new KWidgetAction( hbox, i18n( "Search System Settings" ), 0,
// //                   0, 0, actionCollection(), "search" );
	searchAction->setShortcutConfigurable( false );
// 	searchAction->setAutoSized( true );
	Q3WhatsThis::add( search, i18n( "Search Bar<p>Enter a search term." ) );

	// The Clear search box button.
	QToolButton *clearWidget = new QToolButton(this, QApplication::reverseLayout() ? "clear_left" : "locationbar_erase");
// 	searchClear = new KWidgetAction( clearWidget, QString(""), CTRL+Key_L, search, SLOT(clear()),
// 					actionCollection(), "searchReset");
  searchClear = new KAction ("searchReset", 0);
  searchClear->setShortcut(Qt::CTRL + Qt::Key_L);
  connect(searchClear, SIGNAL(triggered()),
          search, SLOT(clear()));

	connect(clearWidget, SIGNAL(clicked()), searchClear, SLOT(activate()));
	searchClear->setWhatsThis( i18n( "Reset Search\n"
                                        "Resets the search so that "
                                        "all items are shown again." ) );
	//FIXME */

	// Top level pages.
	Q3ValueList<MenuItem> subMenus = menu->menuList();
	Q3ValueList<MenuItem>::iterator it;

  // Now it's time to draw our display
	for ( it = subMenus.begin(); it != subMenus.end(); ++it ) {
		if( (*it).menu ) {
			KServiceGroup::Ptr group = KServiceGroup::group( (*it).subMenu );
			if ( !group ){
				kDebug() << "Invalid Group \"" << (*it).subMenu << "\".  Check your installation."<< endl;
				continue;
			}

			KToggleAction *newAction = new KToggleAction( KIcon(group->icon()), group->caption(), this);
      connect(newAction, SIGNAL(slotToggled(bool)),
              this, SLOT(slotTopPage()));

			pageActions.append(newAction);
      kDebug() << "relpath is :" << group->relPath() << endl;
		}
	}
	//FIXME	pageActions.at(0)->setChecked(true);
}

void MainWindow::aboutCurrentModule()
{
	if(!groupWidget) {
		return;
  }

	KCModuleProxy* module = groupWidget->currentModule();
	if( module && module->aboutData() ){
		KAboutApplicationDialog dlg( module->aboutData() );
		dlg.exec();
	}
}

void MainWindow::groupModulesFinished()
{
	showAllModules();
}

void MainWindow::showAllModules()
{
	windowStack->raiseWidget(moduleTabs);

	// Reset the widget for normal all widget viewing
	groupWidget = 0;
	widgetChange();

	if( embeddedWindows ) {
		showAllAction->setEnabled(false);
	}
	aboutModuleAction->setEnabled(false);

	searchText->setEnabled(true);
	searchClear->setEnabled(true);
	searchAction->setEnabled(true);

	KToggleAction *currentRadioAction;
	for ( currentRadioAction = pageActions.first(); currentRadioAction; currentRadioAction = pageActions.next()) {
		currentRadioAction->setEnabled(true);
	}

	resetModuleHelp();
}

void MainWindow::slotItemSelected( Q3IconViewItem *item ){
	ModuleIconItem *mItem = (ModuleIconItem *)item;

	if( !mItem )
		return;


        kDebug() << "item selected: " << item->text() << endl;
	groupWidget = moduleItemToWidgetDict.find(mItem);
	scrollView = moduleItemToScrollerDict.find(mItem);

	if(groupWidget==0) {
		Q3ValueList<KCModuleInfo> list = mItem->modules;
// 		KDialogBase::DialogType type = KDialogBase::IconList;
// 		if(list.count() == 1) {
// 			type=KDialogBase::Plain;
// 		}

		scrollView = new KCScrollView(windowStack);
		groupWidget = new KCMultiWidget(0, scrollView->viewport(), Qt::NonModal); // THAT ZERO IS NEW (actually the 0 can go, jr)
                scrollView->addChild(groupWidget);
		windowStack->addWidget(scrollView);
		moduleItemToScrollerDict.insert(mItem,scrollView);
		moduleItemToWidgetDict.insert(mItem,groupWidget);

		connect(groupWidget, SIGNAL(aboutToShow( KCModuleProxy * )), this, SLOT(updateModuleHelp( KCModuleProxy * )));
		//FIXME		connect(groupWidget, SIGNAL(aboutToShowPage( QWidget* )), this, SLOT(widgetChange()));
		connect(groupWidget, SIGNAL(finished()), this, SLOT(groupModulesFinished()));
		connect(groupWidget, SIGNAL(close()), this, SLOT(showAllModules()));

		Q3ValueList<KCModuleInfo>::iterator it;
		for ( it = list.begin(); it != list.end(); ++it ){
			qDebug("adding %s %s", (*it).moduleName().latin1(), (*it).fileName().latin1());
			groupWidget->addModule(	*it );
		}
		groupWidget->reparent(scrollView->viewport(), 0, QPoint());
		scrollView->reparent(windowStack, 0, QPoint());
	}

	if( embeddedWindows ) {
		windowStack->raiseWidget( scrollView );

		setCaption( mItem->text() );
		showAllAction->setEnabled(true);
		//searchText->setEnabled(false);
		//searchClear->setEnabled(false);
		//searchAction->setEnabled(false);

		KToggleAction *currentRadioAction;
		for ( currentRadioAction = pageActions.first(); currentRadioAction; currentRadioAction = pageActions.next()) {
			currentRadioAction->setEnabled(false);
		}

	} else {
		scrollView->show();
	}
	groupWidget->show();

	// We resize and expand the window if neccessary, but only once the window has been updated.
	// Some modules seem to dynamically change thier size. The new size is only available
	// once the dialog is updated. :-/ -SBE
	QTimer::singleShot(0,this,SLOT(timerResize()));
}

void MainWindow::timerResize() {
	QSize currentSize = size();
	QSize newSize = currentSize.expandedTo(sizeHint());
	// Avoid resizing if possible.
	if(newSize!=currentSize) {
		resize(newSize);
	}
}

void MainWindow::updateModuleHelp( KCModuleProxy *currentModule ) {
	if ( currentModule->aboutData() ) {
		aboutModuleAction->setText(i18nc("Help menu->about <modulename>", "About %1").arg(
				                             currentModule->moduleInfo().moduleName().replace("&","&&")));
		aboutModuleAction->setIcon(QIcon(currentModule->moduleInfo().icon()));
		aboutModuleAction->setEnabled(true);
	}
	else {
		resetModuleHelp();
	}
}

void MainWindow::resetModuleHelp() {
	aboutModuleAction->setText(i18n("About Current Module"));
	aboutModuleAction->setIconSet(QIcon());
	aboutModuleAction->setEnabled(false);
}

void MainWindow::widgetChange() {
	QString name;
	if( groupWidget && groupWidget->currentModule()) {
		name = groupWidget->currentModule()->moduleInfo().moduleName();
	}

	if( !groupWidget ) {
		setCaption( "" );
		
		ModulesView *modulesView;
		for( modulesView = modulesViewList.first(); modulesView; modulesView = modulesViewList.next()) {
			modulesView->clearSelection();
		}
	}
}

void MainWindow::slotTopPage() {
	KToggleAction *clickedRadioAction = (KToggleAction *)sender();
	selectedPage = pageActions.find(clickedRadioAction);

	KToggleAction *currentRadioAction;
    for ( currentRadioAction = pageActions.first(); currentRadioAction; currentRadioAction = pageActions.next()) {
		currentRadioAction->setChecked(currentRadioAction==clickedRadioAction);
	}

	windowStack->raiseWidget(overviewPages.at(selectedPage));
}

void MainWindow::slotSearchHits(const QString &query, int *hitList, int length) {
	if(query=="") {
		generalHitLabel->setText("");
		advancedHitLabel->setText("");
	} else {
		
		if(length>=1) {
			generalHitLabel->setText(i18nc("%1 hit in General","%1 hits in General",hitList[0]).arg(hitList[0]));
		}
	
		if(length>=2) {
			advancedHitLabel->setText(i18nc("%1 hit in Advanced","%1 hits in Advanced",hitList[1]).arg(hitList[1]));
		}

	}
}

#include "mainwindow.moc"
