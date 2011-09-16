/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/*
*Header For: Plan display base class
*Purpose: to display a map
*
*/
//
// this is the widget for displaying drawings
// 
//
#include <qt.h> 
#include "plancanvas.h"
#include "main.h"
#include "IconSelectDlg.h"
#include "InputDlg.h"
#include "SelectActiveDlg.h"
#include "SetPenStyle.h"
#include "general_defines.h"

#define max(a,b)  (((a) > (b)) ? (a) : (b))
/*
*Function: where did we hit the grapples
*Inputs:none
*Outputs:none
*Returns:none
*/
int Grapple::WhereHit(const QPoint &pt)
{
	IT_IT("Grapple::WhereHit");
	//
	// determine if hit anywhere within the rectangle or within the grapples
	QRect br = GetRect();
	{
		QRect r(br.topLeft().x() ,br.topLeft().y(),GRAPPLE_SIZE,GRAPPLE_SIZE);
		if(r.contains(pt)){	 return TopLeft;}
	};
	{
		QRect r(br.topRight().x() - GRAPPLE_SIZE,br.topLeft().y(),GRAPPLE_SIZE,GRAPPLE_SIZE);
		if(r.contains(pt)){	 return TopRight;}
	};
	{
		QRect r(br.bottomLeft().x(), br.bottomLeft().y() - GRAPPLE_SIZE, GRAPPLE_SIZE, GRAPPLE_SIZE);
		if(r.contains(pt)){	 return BottomLeft;}
	};
	{
		QRect r(br.bottomRight().x() - GRAPPLE_SIZE, br.bottomRight().y() - GRAPPLE_SIZE ,GRAPPLE_SIZE,GRAPPLE_SIZE);
		if(r.contains(pt)){	 return BottomRight;}
	};
	#if 0
	int midy = (br.top() + br.bottom() - GRAPPLE_SIZE)/2;
	{
		QRect r(br.left(),midy,GRAPPLE_SIZE,GRAPPLE_SIZE);
		if(r.contains(pt)){	 return LeftMid;}
	};
	{
		QRect r(br.right() - GRAPPLE_SIZE,midy,GRAPPLE_SIZE,GRAPPLE_SIZE);
		if(r.contains(pt)){	 return RightMid;}
	};
	//
	int midx = (br.left() + br.right() - GRAPPLE_SIZE)/2;
	{
		QRect r(midx,br.top(),GRAPPLE_SIZE,GRAPPLE_SIZE);
		if(r.contains(pt)){	 return TopMid;}
	};
	{
		QRect r(midx,br.bottom() - GRAPPLE_SIZE,GRAPPLE_SIZE,GRAPPLE_SIZE);
		if(r.contains(pt)){	 return BottomMid;}
	};
	#endif
	if(br.contains(pt,TRUE))
	{
		
		return Inside;
	}

	
	return Outside;
};
/*
*Function:PlanWindow
*Inputs:parent, object name,editor mode
*Outputs:none
*Returns:none
*/
PlanWindow::PlanWindow(QWidget *parent, const char *name, bool editormode ) :
QCanvasView((QCanvas *)0, parent,name),currentState(STATE_IDLE),	lastHit(0),   
foreColour(Qt::black), 	backColour(Qt::black),	lineStyle(0), 
fillState(0), 	lineThickness(0),	fontSize(10), 	currentFont(QFont::defaultFont()),
mode(editormode), 	flashState(0) 
{
	IT_IT("PlanWindow::PlanWindow");

	setFocusPolicy(QWidget::StrongFocus);
	//setMouseTracking(TRUE);
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setLineWidth(2);
	setMidLineWidth(3);
	//
	flush(); // setup clean canvas
	QTimer *pTimer = new QTimer(this);
	connect (pTimer, SIGNAL (timeout ()), this, SLOT (Flash()));	// wire up the item selection timer
	pTimer->start(1500);
	//
	(void) new PlanTips(this); // tool tip handler
	
};
/*
*Function: Flash
*flas icons marked as flashable
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::Flash()
{
	IT_IT("PlanWindow::Flash");

	flashState = !flashState;
	QCanvasItemList l = canvas()->allItems();
	QCanvasItemList::Iterator i = l.begin();
	//
	for(; i != l.end(); ++i)
	{
		if((*i)->rtti() == PLAN_ACTIVE) 
		{
			PlanActive *p = (PlanActive *)(*i);  
			if(p->GetFlash())
			{
				if(flashState)
				{
					p->ShowText(); 
				}
				else
				{
					p->HideText(); 
				};
			}
		};
	}; 

	canvas()->update(); 

	
};
/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::resizeEvent(QResizeEvent *p)
{
	IT_IT("PlanWindow::resizeEvent");

	QCanvasView::resizeEvent(p);

	if((p->size().width() > canvas()->width()) || (p->size().height() > canvas()->height()))
	{
		canvas()->resize(max(p->size().width(),canvas()->width()),max(p->size().height(),canvas()->height()));
	};

	
};
/*
*Function: Load
*Load from stream
*Inputs:none
*Outputs:none
*Returns:none
*/
int PlanWindow::Load(QDataStream &is)
{
	IT_IT("PlanWindow::Load");
	//
	flush();
	lineGrapple0->hide();
	lineGrapple1->hide();
	grappleFrame->hide();
	setCursor(arrowCursor);
	//
	// get the object type
	// 
	QString s;
	bool done = 0;
	is >> s;
	if(s != "SCADAPLAN") {	 return 0;} // check the magic number
	//
	while(!done)
	{
		int type;
		is >> type;
		switch(type)
		{
			case PLAN_LINE:
			{
				is >> *( new PlanLine(canvas()));               
			};
			break;
			case PLAN_RECTANGLE:
			{
				is >> *( new PlanRectangle(canvas()));               
			};
			break;
			case PLAN_CIRCLE:
			{
				is >> *( new PlanCircle(canvas()));               
			};
			break;
			case PLAN_ICON:
			{
				IT_COMMENT("PLAN_ICON");

				is >> *( new PlanIcon(canvas()));   //here there is access violation with QT 3.2.1
			};
			break;
			case PLAN_LABEL:
			{
				IT_COMMENT("PLAN_LABEL");

				is >> *(new PlanLabel(canvas()));               
			};
			break;
			case PLAN_ACTIVE:
			{
				IT_COMMENT("PLAN_ACTIVE");

				is >> *( new PlanActive(canvas()));               
			};
			break;
			case PLAN_LINK:
			{
				IT_COMMENT("PLAN_LINK");

				is >> *(new PlanLink(canvas()));               
			};
			break;
			default:
			done = true;
			break;
		}; 
	};
	//
	QCanvasItemList l = canvas()->allItems();
	QCanvasItemList::Iterator i = l.begin();
	//
	for(; i != l.end(); ++i)
	{
		if((*i)->rtti() == PLAN_ACTIVE)
		{
			PlanActive *p = (PlanActive *)(*i);

			if(p->isRanged())
			{
				//do nothing
			}
			else // animated
			{
				(*i)->show(); // show
			};  
		}
		else
		{
			(*i)->show(); // show
		}
	};
	//
	canvas()->update();
	repaint(TRUE); 
	gotoIdle();
	
	return R_SUCCESS;
};
/*
*Function:Save
*Save to a stream
*Inputs:none
*Outputs:none
*Returns:none
*/
// **********
void  PlanWindow::Save(QDataStream &os)
{
	IT_IT("PlanWindow::Save");

	os << QString("SCADAPLAN"); // magic number
	//
	setCursor(arrowCursor);
	QCanvasItemList l = canvas()->allItems();
	QCanvasItemList::Iterator i = l.begin();
	//
	
	for(; i != l.end(); ++i)
	{
		if((*i)->rtti() > PLAN_OBJECT)
		{
			os << (*i)->rtti();
			switch((*i)->rtti())
			{
				case PLAN_LINE:
				{
					os << *((PlanLine *)(*i));               
				};
				break;
				case PLAN_RECTANGLE:
				{
					os << *((PlanRectangle*)(*i));               
				};
				break;
				case PLAN_CIRCLE:
				{
					os << *((PlanCircle*)(*i));               
				};
				break;
				case PLAN_ICON:
				{
					os << *((PlanIcon*)(*i));        
				};
				break;
				case PLAN_LABEL:
				{
					os << *((PlanLabel*)(*i));               
				};
				break;
				case PLAN_ACTIVE:
				{
					os << *((PlanActive*)(*i));               
				};
				break;
				case PLAN_LINK:
				{
					os << *((PlanLink*)(*i));               
				};
				break;
				default:
				// not written (grapple or what ever)
				break;
			};
		};
	}; 

	os << (int)PLAN_END; // terminate the list

	
}; // save a map
/*
*Function: selectObject
*handle object selection
*Inputs:none
*Outputs:none
*Returns:none
*/
bool PlanWindow::selectObject(QMouseEvent *pEvent)
{
	IT_IT("PlanWindow::selectObject");

	QCanvasItemList list = canvas()->collisions(pEvent->pos());
	pselectedObject = 0;
	QCanvasItemList::Iterator i = list.begin();
	if(mode) // are we an editor or display
	{
		lineGrapple0->hide();
		lineGrapple1->hide();
		grappleFrame->hide();
		bool ret = false;
		for(;i != list.end();++i)
		{
			QCanvasItem *p = (*i);		          
			ret = true;
			//cerr << " Hit " << (*i)->rtti() << endl;
			// we want the smallest object or a line
			if(pselectedObject && (pselectedObject->rtti() != PLAN_LINE))
			{
				if(p->rtti() != PLAN_LINE)
				{
					// must be rectangle object
					// so we can compare sizes
					const QRect &r = p->boundingRect();
					const QRect &c = pselectedObject->boundingRect();
					if((c.width() < r.width()) || (c.height() < r.height()))
					{
						continue;
					};
				};
			};
			pselectedObject = p; //
			switch(p->rtti())
			{
				case PLAN_LINE: // ... from Outer Space !!!!
				{
					//
					((PlanLine *)(*i))->GetEndPoints(startPoint,endPoint);
					nextPoint = endPoint;
					currentState =  STATE_SELECTED_LINE;
					//
					// show the line grapples - one at each end
					lineGrapple0->move(startPoint.x(),startPoint.y());
					lineGrapple1->move(endPoint.x(),endPoint.y());
					lineGrapple0->show();
					lineGrapple1->show();
					//
					canvas()->update();
					goto FoundLine;
				};
				break;
				case PLAN_RECTANGLE: // these are all rectangles
				case PLAN_CIRCLE:
				case PLAN_ICON:
				case PLAN_LABEL:
				case PLAN_ACTIVE:	
				case PLAN_LINK:
				{
					//
					const QRect &r = (*i)->boundingRect();
					//
					grappleFrame->Select(r);
					repaint();
					//
					startPoint = QPoint(r.left(),r.top());
					endPoint   = QPoint(r.right(),r.bottom());
					nextPoint  = endPoint;
					//
					currentState = STATE_SELECT;
					//
				};
				break;
				default:
				currentState = STATE_IDLE;
				break;
			};
		};

		FoundLine:
		if(ret) repaint(FALSE);

		
		return ret;
	}
	else
	{
		for(;i != list.end();++i)
		{
			if((*i)) // is this drawable ?
			{
				pselectedObject = (*i); 
				switch((*i)->rtti())
				{
					case PLAN_ACTIVE:
					
					return true;
					//
					case PLAN_LINK:
					
					return true;
					default:
					break;
				};
			};
		};
	};

	currentState = STATE_IDLE;
	
	return false;
};
/*
*Function:DeleteselectedObject
*delete the current object
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::deleteSelectedObject()
{
	IT_IT("PlanWindow::deleteSelectedObject");

	if(pselectedObject)
	{
		pselectedObject->hide();
		grappleFrame->hide();
		lineGrapple0->hide();
		lineGrapple1->hide();
		delete pselectedObject;
		pselectedObject = 0;
		canvas()->update();
	};

	
};
/*
*Function:mousePressEvent
*Inputs:handle mouse clicks
*Outputs:none
*Returns:none
*/
void PlanWindow::contentsMousePressEvent( QMouseEvent *pEvent)
{
	IT_IT("PlanWindow::contentsMousePressEvent");

	QCanvasView::contentsMousePressEvent (pEvent);
	switch(pEvent->button())
	{
		case LeftButton:
		{
			switch(currentState)
			{
				case  STATE_SELECTED_LINE:
				{
					((PlanLine *)pselectedObject)->GetEndPoints(startPoint,endPoint);
					if(lineGrapple0->rect().contains(pEvent->pos()))
					{
						usinglg0 = true; // flag which one we want 
						currentState = STATE_RESIZING_LINE;
						setCursor(sizeAllCursor);
					}
					else if (lineGrapple1->rect().contains(pEvent->pos()))
					{
						usinglg0 = false; // flag which one we want 
						currentState = STATE_RESIZING_LINE;
						setCursor(sizeAllCursor);
					}
					else
					{
						selectAndIdle(pEvent); // no hit
					};
				};
				break;
				case  STATE_SELECT: 
				{
					if(pselectedObject)
					{
						switch(pselectedObject->rtti())
						{
							case PLAN_LINE:
							{
								currentState = STATE_IDLE;
							};
							break;
							case PLAN_ICON: 
							case PLAN_ACTIVE:
							case PLAN_LINK:
							case PLAN_LABEL:
							{
								lastHit = grappleFrame->WhereHit(pEvent->pos());
								switch(lastHit)
								{
									case
									Grapple::Outside:
									{
										selectAndIdle(pEvent);
									};
									break;
									default:
									{
										currentState = STATE_MOVING;
										setCursor(crossCursor);
									};
									break;
								};
							};
							break;
							//
							//case PLAN_ACTIVE:
							case PLAN_RECTANGLE: 
							case PLAN_CIRCLE:
							{
								lastHit = grappleFrame->WhereHit(pEvent->pos());
								//cerr << " Last Grapple Hit " << lastHit << endl;
								switch(lastHit)
								{
									case Grapple::Outside:
									{
										selectAndIdle(pEvent);
									};
									break;
									case Grapple::Inside:
									{
										currentState = STATE_MOVING;
										setCursor(crossCursor);
									};
									break;
									default: 
									{
										currentState = STATE_RESIZE;
										setCursor(sizeAllCursor);
									};
									break;
								};
							};
							break;
							default:
							break;
						};
					};
				};
				break;
				case STATE_IDLE:
				{
					if(!selectObject(pEvent))
					{
						grappleFrame->hide();
						canvas()->update();
						repaint(FALSE);
						setCursor(arrowCursor);
					}
					else if(pselectedObject && (!mode))
					{
						if (pselectedObject->rtti() == PLAN_ACTIVE)
						{
							emit ActiveObjectSelected((PlanActive *)pselectedObject);	
							emit SelectActiveObject(((PlanActive *)pselectedObject)->GetName());
						}
						else if (pselectedObject->rtti() == PLAN_LINK)
						{
							emit LinkSelected((PlanLink *)pselectedObject);
						}
					};
				};
				break;
				case STATE_LINE:   
				{
					//
					startPoint = pEvent->pos();
					endPoint = startPoint + QPoint(30,30);
					nextPoint = endPoint;
					//
					QPen pen(foreColour,lineThickness + 1,(Qt::PenStyle)(Qt::SolidLine + lineStyle));
					//
					pselectedObject = new PlanLine(canvas(),startPoint,endPoint,pen);
					pselectedObject->setZ(1);
					pselectedObject->show();
					//
					currentState =  STATE_SELECTED_LINE;
					//
					// show the line grapples - one at each end
					lineGrapple0->move(startPoint.x(),startPoint.y());
					lineGrapple1->move(endPoint.x(),endPoint.y());
					lineGrapple0->show();
					lineGrapple1->show();
					//
					canvas()->update();
					repaint(FALSE);
					//
				};
				break;
				case STATE_RECT:  
				{
					//cerr << "Adding Rectangle At " << pEvent->pos().x() << " " << pEvent->pos().y() << endl;
					PlanRectangle *p = new PlanRectangle(canvas(),QRect(50,50,100,100));
					QPen pen(foreColour,lineThickness + 1,(Qt::PenStyle)(Qt::SolidLine + lineStyle)); // set the pen style
					p->setPen(pen);
					if(fillState)
					{
						QBrush b(foreColour);
						p->setBrush(b);
					};
					newRectObject(pEvent->pos(),p);
				};
				break;
				case STATE_CIRCLE:
				{
					PlanCircle *p = new PlanCircle(canvas(),100,100);
					QPen pen(foreColour,lineThickness + 1,(Qt::PenStyle)(Qt::SolidLine + lineStyle)); // set the pen style
					p->setPen(pen);
					QBrush b(foreColour);// only have solid circles
					p->setBrush(b);
					newRectObject(pEvent->pos(), p);
					// circles set the centre point
				};
				break;
				case STATE_BITMAP: 
				{
					currentState = STATE_IDLE;
					IconSelectDlg dlg(this);
					if(dlg.exec())
					{
						if(!dlg.GetName().isEmpty())
						{
							newRectObject(pEvent->pos(),new PlanIcon(canvas(),dlg.GetName()));
							pselectedObject->setZ(3);
						}
					}
				};
				break;
				case STATE_TEXT:  
				{
					currentState = STATE_IDLE;
					InputDlg dlg(this);
					if(dlg.exec())
					{
						QString s = dlg.GetText();
						PlanLabel *p = new PlanLabel(canvas(),s, foreColour,currentFont);
						newRectObject(pEvent->pos(),p);
					}
				};
				break;
				case STATE_ACTIVE: 
				{
					currentState = STATE_IDLE;
					SelectActiveDlg dlg(this,this,"","");
					if(dlg.exec())
					{
						QString obj(dlg.GetActive());
						QString bmp(dlg.GetIcon());
						newRectObject(pEvent->pos(),new PlanActive(canvas(),obj,bmp));
						((PlanActive *)pselectedObject)->SetTagName(dlg.GetTagName());
					}
				};
				break;
				case STATE_LINK:   
				{
					currentState = STATE_IDLE;
					//
					QString f =  QFileDialog::getOpenFileName(QSMAPS_DIR + "/*."PLAN_EXT,tr("(Plan Files) *."PLAN_EXT),
					this,"Open",tr("Select Plan To Link To"));
					//
					if(!f.isEmpty())
					{
						newRectObject(pEvent->pos(),new PlanLink(canvas(), f, QString("link")));
					};
				};
				break;
				default:
				{
					repaint(TRUE);
				};
				break;
			};
		};
		break;
		case RightButton:
		{
			currentState = STATE_IDLE;
			if(selectObject(pEvent))
			{
				//
				if(!mode) currentState = STATE_IDLE; // if display only then set the state back
				//
				if(pselectedObject)
				{
					if (pselectedObject->rtti() == PLAN_ACTIVE)
					{
						emit ActiveObjectSelected((PlanActive *)pselectedObject);

						if(pselectedObject)
						{
							emit SelectActiveObject(((PlanActive *)pselectedObject)->GetName());
							emit RightClicked(((PlanActive *)pselectedObject)->GetName());
						}
					}
					else if (pselectedObject->rtti() == PLAN_LINK)
					{
						emit LinkSelected((PlanLink *)pselectedObject);
					}
					else
					{
						if(mode)
						{
							// default 
							deleteSelectedObject();
							gotoIdle();
						};
					};
				}
			};
		};
		break;
		default:
		break;
	};

	
}; 
/*
*Function:mouseReleaseEvent
*handle mouse clicks
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::contentsMouseReleaseEvent( QMouseEvent *pEvent)
{
	IT_IT("PlanWindow::contentsMouseReleaseEvent");

	QCanvasView::contentsMouseReleaseEvent (pEvent);

	if(mode) 
	{
		setCursor(arrowCursor);
		switch(currentState)
		{
			case STATE_RESIZE:  
			{
				if(pselectedObject)
				{
					switch(pselectedObject->rtti())
					{
						case PLAN_RECTANGLE: // these are all rectangles
						{
							pselectedObject->setX(grappleFrame->x());
							pselectedObject->setY(grappleFrame->y());
							((PlanRectangle *)pselectedObject)->setSize(grappleFrame->width(),grappleFrame->height());
						};
						break;
						case PLAN_CIRCLE:
						{
							//
							QPoint c =  grappleFrame->GetRect().center(); 
							pselectedObject->setX(c.x());
							pselectedObject->setY(c.y());
							//
							((PlanCircle *)pselectedObject)->setSize(grappleFrame->width(),grappleFrame->height());
						};
						break;
						default: 
						break;
					};
					currentState = STATE_SELECT;
					canvas()->update();
					repaint();
				};
			};
			break;
			case STATE_RESIZING_LINE:
			{
				if(pselectedObject)
				{
					currentState = STATE_SELECTED_LINE;
					repaint();
				};
			};
			break;
			case STATE_SELECTED_LINE:
			break;
			default:
			{
				if(pselectedObject)
				{
					currentState = STATE_SELECT;
				};
			};
			break;
		};
	};

	
}; 
/*
*Function:mouseMoveEvent
*handle mouse moves 
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::contentsMouseMoveEvent( QMouseEvent *pEvent)
{
	IT_IT("PlanWindow::contentsMouseMoveEvent");

	//
	QCanvasView::contentsMouseMoveEvent(pEvent);
	//
	if(pEvent->state() == LeftButton)
	{
		nextPoint = pEvent->pos();
		switch(currentState)
		{
			case STATE_RESIZING_LINE:
			{
				if(pselectedObject)
				{
					// update the line grapple
					if(usinglg0)
					{
						((PlanLine *)pselectedObject)->SetEndPoints(nextPoint,endPoint);
						lineGrapple0->move(nextPoint.x(),nextPoint.y());
					}
					else
					{
						((PlanLine *)pselectedObject)->SetEndPoints(startPoint,nextPoint);
						lineGrapple1->move(nextPoint.x(),nextPoint.y());
					};
					canvas()->update();
				}; 
			};
			break;
			case STATE_MOVING:  
			{
				if(pselectedObject)
				{
					pselectedObject->hide();
					grappleFrame->hide();
					canvas()->update();
					pselectedObject->move(pEvent->pos().x(),pEvent->pos().y());
					pselectedObject->show();
					grappleFrame->Select(pselectedObject->boundingRect());
					canvas()->update();
					repaint(FALSE);
				};
			};
			break;
			case STATE_RESIZE:  
			{
				QRect br = grappleFrame->GetRect();
				switch(lastHit)
				{
					case Grapple::TopLeft:
					{
						if(nextPoint.x() >  br.right())
						{
							nextPoint.setX(br.right() - 1);
						};
						if(nextPoint.y() > br.bottom())
						{
							nextPoint.setY(br.bottom() - 1);
						};
					};
					break;
					case Grapple::TopRight:
					{
						if(nextPoint.x() <  br.left())
						{
							nextPoint.setX(br.left()+ 1);
						};
						if(nextPoint.y() > br.bottom())
						{
							nextPoint.setY(br.bottom() - 1);
						};
					};
					break;
					case Grapple::BottomLeft:
					{
						if(nextPoint.x() >  br.right())
						{
							nextPoint.setX(br.right() - 1);
						};
						if(nextPoint.y() < br.top())
						{
							nextPoint.setY(br.top() + 1);
						};
					};
					break;
					case Grapple::BottomRight:
					{
						if(nextPoint.x() <  br.left())
						{
							nextPoint.setX(br.left() + 1);
						};
						if(nextPoint.y() < br.top())
						{
							nextPoint.setY(br.top() + 1);
						};
					};
					break;
					case Grapple::TopMid: // only y can vary
					{
						if(nextPoint.y() > br.bottom())
						{
							nextPoint.setY(br.bottom() - 1);
						};
					};
					break;
					case Grapple::BottomMid:
					{
						if(nextPoint.y() < br.top())
						{
							nextPoint.setY(br.top() + 1);
						};
					};
					break;
					case Grapple::LeftMid:
					{
						if(nextPoint.x() >  br.right())
						{
							nextPoint.setX(br.right() - 1);
						};
					};
					break;
					case Grapple::RightMid:
					{
						if(nextPoint.x() <  br.left())
						{
							nextPoint.setX(br.left() + 1);
						};
					};
					break;
					default:
					{
					};
					break;
				};
				//
				endPoint = nextPoint;
				//
				grappleFrame->hide();
				canvas()->setChanged(br);
				switch(lastHit)
				{
					case Grapple::TopLeft: 
					{
						br.setTop(nextPoint.y());
						br.setLeft(nextPoint.x());
					};
					break;
					case Grapple::TopRight:
					{
						br.setTop(nextPoint.y());
						br.setRight(nextPoint.x());
					};
					break;
					case Grapple::BottomLeft:
					{
						br.setBottom(nextPoint.y());
						br.setLeft(nextPoint.x());
					};
					break;
					case Grapple::BottomRight:
					{
						br.setBottom(nextPoint.y());
						br.setRight(nextPoint.x());
					};
					break;
					case Grapple::TopMid:
					{
						br.setTop(nextPoint.y());
					};
					break;
					case Grapple::BottomMid:
					{
						br.setBottom(nextPoint.y());
					};
					break;
					case Grapple::LeftMid:
					{
						br.setLeft(nextPoint.x());
					};
					break;
					case Grapple::RightMid:
					{
						br.setRight(nextPoint.x());
					};
					break;
					default:
					break;
				};
				//
				grappleFrame->move(br.topLeft().x(),br.topLeft().y());
				grappleFrame->setSize(br.width(),br.height());
				grappleFrame->show();
				canvas()->setChanged(br);
				canvas()->update();
				repaint(FALSE);
				//
			};
			break;
			default:
			break;
		};
	};

	
}; 
/*
*Function: setPen
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::setPen()  
{
	IT_IT("PlanWindow::setPen");

	gotoIdle();

	SetPenStyle dlg(this);
	//
	dlg.Thickness->setCurrentItem(lineThickness);
	dlg.Style->setCurrentItem(lineStyle);
	//
	if(dlg.exec())
	{
		lineThickness = dlg.Thickness->currentItem();
		lineStyle = dlg.Style->currentItem();
	};

	
};
/*
*Function: setforeColour
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::setforeColour() 
{
	IT_IT("PlanWindow::setforeColour");

	gotoIdle();

	QColor c = QColorDialog::getColor(foreColour,this);
	if(c.isValid())
	{
		foreColour = c;
	};

	
};
/*
*Function: setbackColour
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::setbackColour() 
{
	IT_IT("PlanWindow::setbackColour");

	gotoIdle();

	QColor c = QColorDialog::getColor(backColour,this);

	if(c.isValid())
	{
		backColour = c;
		canvas()->setBackgroundColor(backColour);
	};

	
};
/*
*Function: setFont
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::setFont() 
{
	IT_IT("PlanWindow::setFont");

	gotoIdle();

	bool ok = 0;

	QFont f = QFontDialog::getFont(&ok,this);

	if(ok)
	{
		currentFont = f; 
	};

	
};
/*
*Function: add Line
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::addLine() 
{
	IT_IT("PlanWindow::addLine");

	pselectedObject = 0;
	currentState = STATE_LINE;
	setCursor(crossCursor);

	
};
/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::addRectangle() 
{
	IT_IT("PlanWindow::addRectangle");

	pselectedObject = 0;
	currentState = STATE_RECT;
	setCursor(crossCursor);
	
};
// **********
// Function:
// Purpose: 
// Inputs:  
// Outputs: none
// Returns: none
// **********
void PlanWindow::addIcon() 
{
	IT_IT("PlanWindow::addIcon");

	pselectedObject = 0;
	currentState = STATE_BITMAP;
	setCursor(crossCursor);
	
};
/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::addLabel() 
{
	IT_IT("PlanWindow::addLabel");

	pselectedObject = 0;
	currentState = STATE_TEXT;
	setCursor(crossCursor);
	
};
/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::addCircle()
{
	IT_IT("PlanWindow::addCircle");

	pselectedObject = 0;
	currentState = STATE_CIRCLE;
	setCursor(crossCursor);
	
};
/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::addActive() 
{
	IT_IT("PlanWindow::addActive");

	pselectedObject = 0;
	currentState = STATE_ACTIVE;
	setCursor(crossCursor);
	
};
/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::addLink() 
{
	IT_IT("PlanWindow::addLink");

	pselectedObject = 0;
	currentState = STATE_LINK;
	setCursor(crossCursor);
	
};
/*
*Function:New
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::newPlan()
{
	IT_IT("PlanWindow::newPlan");

	flush();
	pselectedObject = 0;
	currentState = STATE_IDLE;
	repaint(TRUE);
	
};
/*
*Function: setFillOn
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::setFillOn() 
{
	IT_IT("PlanWindow::setFillOn");
	fillState = true;
	
};
/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::setFillOff()
{
	IT_IT("PlanWindow::setFillOff");
	fillState = false;
	
};
/*
*Function: FindActive
*Inputs:obejct to find
*Outputs:none
*Returns:pointer to object / null
*/
PlanActive * PlanWindow::FindActiveObject(const QString &n)
{
	IT_IT("PlanWindow::FindActiveObject");

	if(canvas())
	{
		QCanvasItemList l = canvas()->allItems();
		QCanvasItemList::Iterator i = l.begin();
		//
		for(; i != l.end(); ++i)
		{
			//IT_COMMENT1("(*i)->rtti() = %d", (*i)->rtti());

			if((*i)->rtti() == PLAN_ACTIVE) 
			{
				PlanActive *p = (PlanActive *)(*i);

				//IT_COMMENT2("p->GetName() = %s, n = %s", (const char*)p->GetName(), (const char*)n);

				if(p->GetName() == n)
				{
					
					return p;  
				};
			};
		};
	};

	
	return 0;
};
/*
*Function:UpdateActiveObject
*Inputs:object name, new value string, new colour
*Outputs:none
*Returns:none
*/
void PlanWindow::UpdateActiveObject(const QString &name,const QString &Value, const QColor c)
{
	IT_IT("PlanWindow::UpdateActiveObject");

	PlanActive *p = FindActiveObject(name);

	if(p)
	{
		p->SetValue(Value);
		p->SetColour(c);
		repaint(false);
	};

	
};
/*
*Function:UpdateActiveValue
*Inputs:object name, new value string
*Outputs:none
*Returns:none
*/
void PlanWindow::UpdateActiveValue(const QString &name,const QString &tag,const QString &Value)
{
	IT_IT("PlanWindow::UpdateActiveValue");

	PlanActive *p = FindActiveObject(name);

	if(p) 
	{
		if(p->GetTagName() == tag)
		{
			p->SetValue(Value);
		};
	};

	
};
/*
*Function:UpdateActiveColour
*Inputs:object name, new colour
*Outputs:none
*Returns:none
*/
void PlanWindow::UpdateActiveColour(const QString &name, QColor c)
{
	IT_IT("PlanWindow::UpdateActiveColour");

	PlanActive *p = FindActiveObject(name);

	if(p)
	{
		p->SetColour(c);
		repaint(false);
	};

	
};
/*
*Function:UpdateActiveFlash
*update the flash state of an object
*Inputs:object name, new colour
*Outputs:none
*Returns:none
*/
void PlanWindow::UpdateActiveFlash(const QString &name, bool f)
{
	IT_IT("PlanWindow::UpdateActiveFlash");

	PlanActive *p = FindActiveObject(name);

	if(p)
	{
		p->SetFlash(f);
	};

	
};
/*
*Function: Print
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::Print() 
{
	IT_IT("PlanWindow::Print");

	lineGrapple0->hide();
	lineGrapple1->hide();
	grappleFrame->hide();
	((PlanCanvas *)canvas())->Print(this);

	
};
/*
*Function: flush
*clear the canvas
*Inputs:none
*Outputs:none
*Returns:none
*/
void PlanWindow::flush()
{
	IT_IT("PlanWindow::flush");

	if(canvas())
	{
		QCanvasItemList l = canvas()->allItems();
		QCanvasItemList::Iterator i = l.begin();
		//
		for(; i != l.end(); ++i)
		{
			if((*i)) delete (*i); // delete everything
		};
		//
		delete canvas(); // delete the old canvas
	};
	//
	setCanvas(new PlanCanvas(this));
	grappleFrame = new Grapple(canvas());
	//
	QRect l(0,0,5,5);
	lineGrapple0 = new QCanvasRectangle(l,canvas());
	lineGrapple1 = new QCanvasRectangle(l,canvas());
	//
	//canvas()->setBackgroundColor(HMI_BACKGROUND_COLOR);
	//backColour = HMI_BACKGROUND_COLOR; 
	canvas()->setBackgroundColor(backColour);
	//
	canvas()->resize(width(),height());
	canvas()->update();
	//
	QTimer *pTicks = new QTimer(this);
	connect (pTicks, SIGNAL (timeout ()), canvas(),SLOT(advance()));	// wire up the item selection timer
	pTicks->start(500);
	//
	repaint(TRUE); 

	
};
/*
*Function:FitRect
*only display if the text fist on the screen
*Inputs:text, window size
*Outputs:none
*Returns:none
*/
QRect PlanTips::FitRect(const QString &t, const QRect &r) // workout what fits
{
	IT_IT("PlanTips::FitRect");
	//
	QRect res;
	QFontMetrics fmt(font());
	//
	int w = fmt.width(t); 
	int h = fmt.height();
	//
	// we must no adjust the position so the rectangle fits
	//
	if((pP->width() > w) && (pP->height() > h)) // is it possible to fit it
	{
		int l = r.left();
		if(r.left() + w > pP->width())
		{
			l = (pP->width() - w);
		};		
		int y = r.top();
		if(r.top() + h > pP->height())
		{
			y = pP->height() - h;
		};	
		res = QRect(l,y,w,h);
	};

	
	return res;
};
/*
*Function:maybeTip
*Inputs:Current mouse position
*Outputs:none
*Returns:none
*/
void PlanTips::maybeTip(const QPoint &pt) // handle tip popup
{
	IT_IT("PlanTips::maybeTip");

	QCanvasItemList l = pP->canvas()->collisions(pP->viewportToContents(pt)); // get the list of selected objects
	QCanvasItemList::Iterator i = l.begin();
	//
	for(; i != l.end(); ++i)
	{
		if( ((*i)->rtti() == PLAN_ACTIVE) || ((*i)->rtti() == PLAN_LINK))
		{
			// create the tip - the name of either the link or the active object
			if((*i)->rtti() == PLAN_ACTIVE)
			{
				QString t = ((PlanActive *)(*i))->GetName();
				QRect r = FitRect(t,((PlanActive *)(*i))->boundingRect());
				if(!r.isEmpty())
				{
					tip(r,t);
				};
			}
			else if((*i)->rtti() == PLAN_LINK)
			{
				QString t = ((PlanLink *)(*i))->GetName();
				QRect r = FitRect(t,((PlanLink *)(*i))->boundingRect());
				if(!r.isEmpty())
				{
					tip(r,t);
				};
			};
			break;			 
		};
	};

	
}
//
//
//
/*
*Function:GetImageArray
*Inputs:definition file name
*Outputs:none
*Returns:pixmap 
*/
PlanIcon::IconMap PlanIcon::Icons; // dictionary of icons
//
//
//
static void GenerateDefaultIcon() // create the default icon
{
	IT_IT("GenerateDefaultIcon");

	QFile f(QSBMP_DIR + "/default.def"); // get the definiton file
	if(f.open(IO_WriteOnly))
	{
		QTextStream os(&f);
		os << "0" << endl << "default.xpm" << endl << " 0 0 0 " << endl;
	};
	QFile fi(QSBMP_DIR + "/default.xpm");
	if(fi.open(IO_WriteOnly))
	{
		QTextStream os(&fi);
		os << "/""* XPM *""/" << endl
		<< "static char*defaulticon[]={" << endl
		<< "\"22 22 9 1\"," << endl
		<< "\"d c #a0a0a0\"," << endl
		<< "\"e c #00ff00\"," << endl
		<< "\". c #000000\"," << endl
		<< "\"f c #ffff00\"," << endl
		<< "\"b c #c0ffff\"," << endl
		<< "\"a c #585858\"," << endl
		<< "\"# c #ffffff\"," << endl
		<< "\"g c #ff0000\"," << endl
		<< "\"c c #00c0c0\"," << endl
		<< "\".#####################\"," << endl
		<< "\".abbbbbbbbbbbbbbbbbbb#\"," << endl
		<< "\".accccccccccccccccccb#\"," << endl
		<< "\".accccccccccccccccccb#\"," << endl
		<< "\".accccc.......dcccccb#\"," << endl
		<< "\".accccc.eeeee.dcccccb#\"," << endl
		<< "\".accccc.eeeee.dcccccb#\"," << endl
		<< "\".accccc.eeeee.dcccccb#\"," << endl
		<< "\".accccc.......dcccccb#\"," << endl
		<< "\".accccc.fffff.dcccccb#\"," << endl
		<< "\".accccc.fffff.dcccccb#\"," << endl
		<< "\".accccc.fffff.dcccccb#\"," << endl
		<< "\".accccc.......dcccccb#\"," << endl
		<< "\".accccc.ggggg.dcccccb#\"," << endl
		<< "\".accccc.ggggg.dcccccb#\"," << endl
		<< "\".accccc.ggggg.dcccccb#\"," << endl
		<< "\".accccc.......dcccccb#\"," << endl
		<< "\".accccc.......dcccccb#\"," << endl
		<< "\".acccccddddddddcccccb#\"," << endl
		<< "\".accccccccccccccccccb#\"," << endl
		<< "\".aaaaaaaaaaaaaaaaaaab#\"," << endl
		<< "\".....................#\"};" << endl;
	};

	
};

QCanvasPixmapArray * PlanIcon::GetImageArray(const QString &s)
{
	IT_IT("PlanIcon::GetImageArray");
	
	if(!s.isEmpty())
	{
		IconMap::iterator i = Icons.find(s);

		if(!(i == Icons.end()))
		{
			return (*i).second.pArray;
		};
		//
		QFile f(QSBMP_DIR + "/" + s + ".def"); // get the definiton file

		QString file = QSBMP_DIR + "/" + s + ".def";
		IT_COMMENT1("Open definition file %s", (const char *)file);

		if(f.exists())
		{
			//
			if(f.open(IO_ReadOnly))
			{
				QTextStream is(&f);
				int n; // number of images
				QString fname; // the file pattern 
				//
				is >> n;
				is >> ws; // strip white space
				is >> fname; // get the name
				is >> ws;
				//
				//
				QCanvasPixmapArray *p = new QCanvasPixmapArray;

				if(p->readPixmaps(QSBMP_DIR + "/" + fname,n)) // force load from bitmap directories
				{
					IconData d;

					if(!is.atEnd())
					{
						is >> d.Ranged >> d.iMin >> d.iMax; // get the ranging bits
					};

					d.pArray = p;
					Icons.insert(IconMap::value_type(s,d)); 
					//d.pArray = 0;
					
					return p;
				};

				if(p)
				{
					#if (QT_VERSION < 0x030201)
					delete p; //BUG somewhere here, commented for QT version 321  //apa 16-09-2011
					#endif
					p = NULL;
				}
			};
		};

		// failed so load the default icon
		// does it exist ?
		// 

		{
			QFile f(QSBMP_DIR + "/default.def"); // get the definiton file
			if(!f.exists())
			{
				//cerr << " Creating Default Icon Files " << endl;
				GenerateDefaultIcon(); // create the default icon
			};
			QCanvasPixmapArray *p = new QCanvasPixmapArray;
			//cerr << " About to load Default for " << (const char *)s << endl;	
			if(p->readPixmaps(QSBMP_DIR + "/default.xpm",0)) // force load from bitmap directories
			{
				//cerr << "Loaded Default" << endl;
				IconData d;
				d.pArray = p;
				Icons.insert(IconMap::value_type(s,d)); 
				//d.pArray = 0;
				
				return p;
			};
			//cerr << "Failed to Load Default Icon File " << endl;
			if(p)
			{
				delete p;
				p = NULL;
			}
		};

		//
		// Must have crap installation
		//
		QMessageBox::critical(0,"Serious Error","Cannot Access Bitmap Directory To Create Default Icon - Aborting");
		qApp->quit(); // exit the application
		//
	};

	
	return 0;
};

