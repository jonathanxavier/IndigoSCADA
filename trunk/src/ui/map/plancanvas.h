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
#ifndef  PLANWINDOW_HPP
#define  PLANWINDOW_HPP
//
// this is the map display class
//
#include <qt.h>
#include "common.h"
#include "general_defines.h"
#include "sptypes.h"
#include "IndentedTrace.h"
//
// sensitivity for end of line selection
// 
#define  PLANWINDOW_MIN_LINE_DIST 20
#define PLAN_EXT "qsc"
// sensitivity for selecting lines
#define PLAN_LINE_SELECT_MAX 5
//
// this is the header that defines the map object classes
//
enum
{
	PLAN_OBJECT = 10000,
	PLAN_LINE,	PLAN_RECTANGLE,	PLAN_CIRCLE,	PLAN_ICON,	PLAN_LABEL,
	PLAN_ACTIVE,	PLAN_LINK,  PLAN_METER,PLAN_GRAPPLE, PLAN_END,PLAN_N_TYPES         
};
//
#define PLAN_ACTIVE_TEXTY 20
//
inline QDataStream & readCanvasItem(QDataStream &is, QCanvasItem &c)
{
	double x,y,z;
	is >> x >> y >> z;
	c.setX(x);
	c.setY(y);
	c.setZ(z);
	return is;
};
inline QDataStream & writeCanvasItem(QDataStream &is, const QCanvasItem &c)
{
	is << c.x() <<  c.y() << c.z();
	return is;
};
class PlanLine : public QCanvasLine
{
	public:
	PlanLine(QCanvas *c,const QPoint &s = QPoint(0,0), const QPoint &e = QPoint(10,10), const QPen &p = QPen(Qt::black)) : QCanvasLine(c)
	{
		setPen(p);
		setPoints(s.x(),s.y(),e.x(),e.y());
	};
	void SetEndPoints(const QPoint &s, const QPoint &e)
	{
		setPoints(s.x(),s.y(),e.x(),e.y());
	};
	void GetEndPoints(QPoint &s,QPoint &e)
	{
		s = startPoint();
		e = endPoint();
	};
	int rtti() const { return PLAN_LINE;};
	friend QDataStream & operator << (QDataStream &, const PlanLine &);
	friend QDataStream & operator >> (QDataStream &, PlanLine &);
};
inline QDataStream & operator << (QDataStream &os, const PlanLine &l)
{
	os << l.startPoint() << l.endPoint() << l.pen();
	return os;
};
inline QDataStream & operator >> (QDataStream &is, PlanLine &l)
{
	QPoint s,e;
	QPen p;
	is >> s >> e >> p;
	l.setPen(p);
	l.SetEndPoints(s,e);
	l.setZ(1); // lines are level 1
	return is;
};
class PlanRectangle: public QCanvasRectangle
{
	public:
	PlanRectangle(QCanvas *c, const QRect &r = QRect(0,0,1,1)) : QCanvasRectangle(r,c) 
	{
	};
	int rtti() const { return PLAN_RECTANGLE;};
	friend QDataStream & operator << (QDataStream &, const PlanRectangle &);
	friend QDataStream & operator >> (QDataStream &, PlanRectangle &);
};
inline QDataStream & operator << (QDataStream &os, const PlanRectangle &l)
{
	os << l.x() << l.y() << l.width() << l.height() << l.pen() << l.brush();
	return os;
};
inline QDataStream & operator >> (QDataStream &is, PlanRectangle &l)
{
	double x,y;
	int w,h;
	QPen p;
	QBrush b;
	is >> x >> y >> w >> h >> p >> b;
	//
	l.setSize(w,h);
	l.setX(x);
	l.setY(y);
	l.setZ(2); // rectangles are level 2
	//
	l.setPen(p);
	l.setBrush(b);
	return is;
};
class PlanCircle : public QCanvasEllipse
{
	public:
	PlanCircle(QCanvas *c, int w = 1, int h = 1) : QCanvasEllipse(w,h,c) 
	{
	};
	int rtti() const { return PLAN_CIRCLE;};
	friend QDataStream & operator << (QDataStream &, const PlanCircle &);
	friend QDataStream & operator >> (QDataStream &, PlanCircle &);
};
inline QDataStream & operator << (QDataStream &os, const PlanCircle &l)
{
	os << l.x() << l.y() << l.width() << l.height() << l.pen() << l.brush();
	return os;
};
inline QDataStream & operator >> (QDataStream &is, PlanCircle &l)
{
	double x,y;
	int w,h;
	QPen p;
	QBrush b;
	is >> x >> y >> w >> h >> p >> b;
	l.setSize(w,h);
	l.setPen(p);
	l.setBrush(b);
	l.setX(x);
	l.setY(y);
	l.setZ(2); // rectangles are level 2
	return is;
};
class PlanIcon : public QCanvasSprite
{
	protected:
	//
	QString Name;        // name of image array
	double iValue;       // the current value if a ranged icon
	//
	//
	class  IconData // icon data - eventually hook in stuff to make this clever
	{
		public:
		QCanvasPixmapArray *pArray; // array
		double iMin,iMax;      // range if not animated - ranged icon
		// this is a ranged icon - displayed frame varies with value
		//12-08-09 
		//Se e' una icona a sequenza di frame (tipo fiammata del forno), settare a 0 nel file .def
		//Se e' una icona a sequenza di frame che dipendono dal valore analogico (tipo livello vasca), settare a 1 nel file .def         
		//Se e' una icona a sequenza di frame che dipendono dal valore digitale (punto singolo o doppio), settare a 2 nel file .def                  
		int Ranged; 
		
		IconData() :pArray(0),iMin(0),iMax(0),Ranged(0) {};
		~IconData() {}; // assume only delete on close down - shared data
		IconData(const IconData &d) : pArray(d.pArray),iMin(d.iMin),iMax(d.iMax),Ranged(d.Ranged)
		{
		};
	};
	//
	class  IconMap : public std::map<QString,IconData, std::less<QString> > 
	{
		public:
		IconMap() {};
		virtual ~IconMap()
		{
			IT_IT("IconMap::~IconMap");
			
			IconMap::iterator i = begin();
			for(;!(i== end());i++)
			{
				if((*i).second.pArray) 
				{
					delete (*i).second.pArray;
				}
			}
		};
	};
	//
	static IconMap Icons; // map of pixmaps
	//
	void advance(int i) //aggiorna la pixmap
	{
		IT_IT("PlanIcon::advance");

		if(!i)
		{
			if(frameCount() > 1)
			{
				IT_COMMENT1("frameCount() = %d", frameCount());

				IconMap::iterator i = Icons.find(Name);  // look up the table

				IT_COMMENT1("Name = %s", (const char*)Name);

				if(i != Icons.end())
				{
					if((*i).second.Ranged == 1) // ranged analog display, i.e termometer
					{
						IT_COMMENT3("iValue = %f, (*i).second.iMin = %f, (*i).second.iMax = %f", iValue, (*i).second.iMin, (*i).second.iMax);

						if(((*i).second.iMin != (*i).second.iMax))
						{
							int f = int( (iValue - (*i).second.iMin) * ((double)frameCount()) / ((*i).second.iMax - (*i).second.iMin) + 0.5 );

							IT_COMMENT1("Frame calcolato = %d", f);
							//
							if(f != frame())
							{
								if(f < 0) f = 0; // clip
								if(f >=  frameCount()) f = frameCount() - 1;

								IT_COMMENT1("Set frame = %d", f);

								setFrame(f);

								this->show(); //Visualiszzo la pixmap solo dopo che e' stata selezionata
							}
						}
					}
					else if((*i).second.Ranged == 2) // ranged digital display, i.e interruttore: led rosso per CHIUSO oppure led verde per APERTO
					{
						IT_COMMENT1("Digital iValue = %f", iValue);

						int f;

						if(iValue == 0x00){ f = 0; }
						if(iValue == 0x01){ f = 1; }
						if(iValue == 0x02){ f = 2; }
						if(iValue == 0x03){ f = 3; }

							IT_COMMENT1("Frame calcolato = %d", f);
							//
							//if(f != frame()) //commented out on 05-01-2011 MOMENTANEAMENTE, per visualizzare l'icona all'avvio
							{
								//if(f < 0) f = 0; // clip
								//if(f >=  frameCount()) f = frameCount() - 1;

								IT_COMMENT1("Set frame = %d", f);

								setFrame(f);

								this->show(); //Visualiszzo la pixmap solo dopo che e' stata selezionata
							};
					}
					else // animated. Servono a simulare gli oggetti un rotazione
					{
						if(frame() <  (frameCount() - 1))
						{
							setFrame(frame() + 1);

							IT_COMMENT1("Set frame = %d", frame() + 1);
						}
						else
						{
							setFrame(0);
							IT_COMMENT1("Set frame = %d", 0);
						}        
					}  
				}
			}
		}
		else
		{
			QCanvasSprite::advance(i);
		};
	};	
	//
	public:

	PlanIcon(QCanvas *c, const QString &name = QString::null, int x = 0, int y = 0) : 
	QCanvasSprite(PlanIcon::GetImageArray(name),c),
	Name(name)
	{
		IT_IT("PlanIcon::PlanIcon");
		setX(x);
		setY(y);
		setAnimated(TRUE); // multi frame sprites are animated
	};

	int rtti() const { return PLAN_ICON;};

	bool isRanged()
	{
		IT_IT("PlanIcon::isRanged");

		if(frameCount() > 1)
		{
			IT_COMMENT1("frameCount() = %d", frameCount());

			IconMap::iterator i = Icons.find(Name);  // look up the table

			IT_COMMENT1("Name = %s", (const char*)Name);

			if(i != Icons.end())
			{
				if((*i).second.Ranged) // ranged display
				{
					return true;
				}
			};
		};	

		return false;
	}

	static QCanvasPixmapArray * GetImageArray(const QString &);
	void SetIconName(const QString &s) { Name = s;};
	friend QDataStream & operator << (QDataStream &, const PlanIcon &);
	friend QDataStream & operator >> (QDataStream &, PlanIcon &);
};

inline QDataStream & operator << (QDataStream &os, const PlanIcon &l)
{
	os << l.Name << l.x() << l.y();
	return os;
};

inline QDataStream & operator >> (QDataStream &is, PlanIcon &l)
{
	double x, y;
	is >> l.Name >> x >> y;
	l.setSequence(PlanIcon::GetImageArray(l.Name)); //BUG BUG:qui c'e' un problema di carico icona 17-03-2002
	l.setX(x);
	l.setY(y);
	l.setZ(3); // icons are at level 3
	return is;
};
class PlanLabel : public QCanvasText
{
	public:
	PlanLabel(QCanvas *c, const QString &t = "", const QColor clr = Qt::black, const QFont &f = QFont::defaultFont()) : 
	QCanvasText(t,f,c)
	{
		IT_IT("PlanLabel::PlanLabel");
		setColor(clr);
	};
	int rtti() const { return PLAN_LABEL;};
	friend QDataStream & operator << (QDataStream &, const PlanLabel &);
	friend QDataStream & operator >> (QDataStream &, PlanLabel &);
};
inline QDataStream & operator << (QDataStream &os, const PlanLabel &l)
{
	os << l.text() << l.color() << l.font() << l.x() << l.y();
	return os;
};
inline QDataStream & operator >> (QDataStream &is, PlanLabel &l)
{
	QString t;
	QColor c;
	QFont f;
	double x,y;
	is >> t >> c >> f >> x >> y;
	l.setText(t);
	l.setColor(c);
	l.setFont(f);
	l.setX(x);
	l.setY(y);
	l.setZ(3);
	return is;
};
//
//
//
class PlanActive : public PlanIcon
{
	QString NameObject;  // the samplle point associated with this object
	QString TagName;     // the name of the tag for the value
	QString Value;       // the current value
	bool fFlash;         // are we flashing this
	QCanvasText *text;   // the associated text box - only active when SetValue called - only in non editor mode
	//
	public:
	//
	PlanActive( QCanvas *c, const QString &nameObject = QString::null, 
	const QString &nameIcon = QString::null, int x = 1, int y = 1)
	: PlanIcon(c,nameIcon,x,y), NameObject(nameObject),TagName(VALUE_TAG),Value("***"),fFlash(0),text(0)
	{
		IT_IT("PlanActive::PlanActive");
	};
	//virtual void draw(QPainter &dc); 
	virtual int rtti() const { return PLAN_ACTIVE;};
	const QString & GetName() const { return NameObject;};
	//
	void SetName(const QString &s) { NameObject = s;};
	void SetTagName(const QString &s) { TagName = s;};
	void HideText() { if(text) text->hide();};
	void ShowText() { if(text) text->show();};
	virtual void SetValue(const QString &s) 
	{
		IT_IT("PlanActive::SetValue");

		//IT_COMMENT1("text = %p", text);

		Value = s;

		if(!text)
		{
			QFont f("helvetica",10);
			text = new QCanvasText("",f,canvas());
			text->move(x(),y() + height() + 2);
			text->show();
		};

		text->setText(s); // caller calls update
		canvas()->update();
		//
		bool ok = 0;
		double v = Value.toDouble(&ok);
		if(ok)
		{
			iValue = v; // update the associated icon index
		};
		//
	};
	const QString & GetValue() { return Value;};
	void SetFlash(bool f) 
	{
		IT_IT("PlanActive::SetFlash");

		fFlash = f;
		if(!f && text)
		{
			text->show();
		};
	};
	bool GetFlash() const { return fFlash;};
	const QString & GetTagName() const { return TagName;};
	void SetColour(const QColor &c) 
	{
		IT_IT("PlanActive::SetColour");

		if(text)
		{
			text->setColor(c); // set the colour of the text
			canvas()->update();
		};
	};
	friend QDataStream & operator << (QDataStream &, const PlanActive &);
	friend QDataStream & operator >> (QDataStream &, PlanActive &);
};
inline QDataStream & operator << (QDataStream &os, const PlanActive &l)
{
	os << l.NameObject << l.TagName;
	os << l.Name << l.x() << l.y();
	return os;    
};
inline QDataStream & operator >> (QDataStream &is, PlanActive &l)
{
	is >> l.NameObject >> l.TagName;
	double x, y;
	is >> l.Name >> x >> y;
	l.setSequence(PlanIcon::GetImageArray(l.Name));
	l.setX(x);
	l.setY(y);
	l.setZ(3);
	return is;
};
//
//
class PlanLink: public PlanIcon 
{
	QString NameLink;
	public:
	PlanLink(QCanvas *c,  const QString &nameLink ="", const QString &nameIcon="link", int x = 1, int y = 1)
	: PlanIcon(c,nameIcon,x,y),NameLink(nameLink){};
	virtual int rtti() const { return PLAN_LINK;};
	const QString & GetName() const { return NameLink;};
	void SetName(const QString &s) { NameLink = s;};
	friend QDataStream & operator << (QDataStream &, const PlanLink &);
	friend QDataStream & operator >> (QDataStream &, PlanLink &);
};
inline QDataStream & operator << (QDataStream &os, const PlanLink &l)
{
	os << l.NameLink;
	os << l.Name << l.x() << l.y();
	return os;
};
inline QDataStream & operator >> (QDataStream &is, PlanLink &l)
{
	is >> l.NameLink;
	double x, y;
	is >> l.Name >> x >> y;
	l.setSequence(PlanIcon::GetImageArray(l.Name));
	l.setX(x);
	l.setY(y);
	l.setZ(3);
	return is;
};
#define GRAPPLE_SIZE 8
class Grapple : public QCanvasRectangle 
{
	public:
	Grapple(QCanvas *c): QCanvasRectangle(QRect(0,0,10,10), c) 
	{
		IT_IT("Grapple::Grapple");
		setPen(QPen(Qt::black));
	}; 
	// 
	void DrawGrapple(QPainter &dc, const QPoint &pt, const QPoint &d)
	{
		IT_IT("Grapple::DrawGrapple");
		QRect r(pt,QSize(GRAPPLE_SIZE * d.x(), GRAPPLE_SIZE * d.y())); 
		dc.drawRect(r);
	};
	void Select(const QRect &r)
	{
		IT_IT("Grapple::Select");
		move(r.topLeft().x(),r.topLeft().y());
		setSize(r.width()-2,r.height()-2);
		setZ(10);
		show();
		canvas()->update();
	};
	QRect GetRect() const 
	{
		IT_IT("Grapple::GetRect");
		// return the internal size of the rectangle
		QRect r(QPoint((int)x(),(int)y()),size());
		return r;
	};
	//
	void drawShape(QPainter &dc) 
	{
		IT_IT("Grapple::drawShape");
		QCanvasRectangle::drawShape(dc);
		dc.save();
		QRect r = GetRect();
		dc.setPen(Qt::white);
		dc.setRasterOp(Qt::XorROP);
		dc.setBrush(Qt::white);
		//
		DrawGrapple(dc,r.topLeft(),QPoint(1,1));
		DrawGrapple(dc,r.topRight(),QPoint(-1,1));
		DrawGrapple(dc,r.bottomLeft(),QPoint(1,-1));
		DrawGrapple(dc,r.bottomRight(),QPoint(-1,-1));
		//
		#if 0
		int midy = (r.top() + r.bottom() - GRAPPLE_SIZE)/2;
		DrawGrapple(dc, QPoint(r.left(), midy), QPoint(1,-1));
		DrawGrapple(dc, QPoint(r.right(),midy), QPoint(-1,-1));
		//
		int midx = (r.left() + r.right() - GRAPPLE_SIZE)/2;
		DrawGrapple(dc,QPoint(midx, r.top()),QPoint(-1,1));
		DrawGrapple(dc,QPoint(midx, r.bottom()),QPoint(-1,-1));
		#endif
		//
		dc.restore();
	}; 
	enum
	{
		TopLeft=0,		TopRight,		BottomLeft,		BottomRight,		TopMid,
		BottomMid,		LeftMid,		RightMid,		Inside,		Outside
	};
	int WhereHit(const QPoint &); 
};


class PlanCanvas : public QCanvas
{
	public:
	PlanCanvas(QObject *parent) : QCanvas(parent) 
	{
		IT_IT("PlanCanvas::PlanCanvas");
	};

	void Print(QWidget *p)
	{
		IT_IT("PlanCanvas::Print");

		QPrinter prt;

		if(prt.setup(p))
		{
			prt.setDocName(tr("Plan"));
			prt.setCreator(tr(SYSTEM_NAME));
			prt.setOrientation(QPrinter::Landscape);
			//
			QPainter p;
			p.begin(&prt);
			//
			p.setWindow(QRect(QPoint(0,0),size())); // this is the bit to display
			//
			// draw each of the objects
			//
			QCanvasItemList l = allItems();
			QCanvasItemList::Iterator i = l.begin();
			//
			for(; i != l.end(); ++i)
			{
				if((*i)->visible()) (*i)->draw(p); // show everything
			};
			p.end();
		};
	};
};
class PlanWindow : public QCanvasView // the map window
{
	Q_OBJECT
	protected:    
	enum
	{
		STATE_IDLE = 0,	STATE_LINE,   	STATE_RECT,   	STATE_CIRCLE, 	STATE_BITMAP, 	STATE_TEXT,   	STATE_ACTIVE,
		STATE_LINK,   STATE_MOVING, 	STATE_RESIZE,  	STATE_SELECT, 	STATE_SELECTED_LINE, 	STATE_RESIZING_LINE 
	};
	//
	int currentState; 
	int lastHit;   
	QCanvasItem  *pselectedObject; 
	//
	QPoint startPoint;  
	QPoint endPoint;   
	QPoint nextPoint;  
	Grapple *grappleFrame; 
	QCanvasRectangle *lineGrapple0, *lineGrapple1;
	bool usinglg0; // which line grapple we are using
	//
	bool selectObject(QMouseEvent *pEvent); 
	//
	void Draw(QPainter &,const QRect &);
	//
	QColor foreColour; 
	QColor backColour; 
	int  lineStyle; 
	bool fillState; 
	int lineThickness;
	int fontSize; 
	//
	QFont currentFont;
	bool mode; 
	//
	bool flashState; 
	//
	protected:
	//
	void contentsMousePressEvent ( QMouseEvent *); 
	void contentsMouseReleaseEvent ( QMouseEvent *); 
	void contentsMouseMoveEvent ( QMouseEvent *) ; 
	void resizeEvent(QResizeEvent *);
	//
	void gotoIdle() 
	{  
		IT_IT("PlanWindow::gotoIdle");

		repaint(TRUE); // redraw - post a repaint request
		pselectedObject = 0;
		currentState = STATE_IDLE;
		lineGrapple0->hide();
		lineGrapple1->hide();
		grappleFrame->hide();
		canvas()->update();
		setCursor(arrowCursor);
	};
	//
	void clipFrame()
	{
		IT_IT("PlanWindow::clipFrame");

		QRect r = grappleFrame->boundingRect();

		if(r.left() < 0)
		{
			grappleFrame->setX(0);
		};
		if(r.top() < 0)
		{
			grappleFrame->setY(0);
		};
		if(r.right() > width() )
		{
			grappleFrame->setSize(width(),grappleFrame->height());
		};
		if(r.bottom() > height())
		{
			grappleFrame->setSize(grappleFrame->width(),height());
		};
	};
	//
	public:
	//
	PlanWindow(QWidget *parent, const char *name, bool editormode=false );
	~PlanWindow()
	{
		IT_IT("PlanWindow::~PlanWindow");
		flush();
	};
	//
	int Load(QDataStream &); 
	void Save(QDataStream &); 
	//
	PlanActive * FindActiveObject(const QString &);
	//
	void UpdateActiveObject(const QString &name,const QString &Value, const QColor c);
	void UpdateActiveValue(const QString &name,const QString &tag, const QString &Value);
	void UpdateActiveColour(const QString &name, QColor c);
	void UpdateActiveFlash(const QString &name, bool f);
	//
	void deleteSelectedObject();
	//
	void newRectObject(const QPoint &pos,QCanvasItem *p) // add a new rectrangular object
	{
		IT_IT("PlanWindow::newRectObject");

		startPoint = pos;
		endPoint = startPoint + QPoint(40,40);
		QRect r(startPoint,endPoint);
		r = r.normalize();
		pselectedObject = p;
		p->setZ(2); // covers lines but not active objects
		p->move(pos.x(),pos.y());
		p->show();
		grappleFrame->Select(p->boundingRect());
		//
		currentState = STATE_SELECT;
		canvas()->update();
		repaint(FALSE);
	};
	//
	void selectAndIdle(QMouseEvent *pEvent)
	{
		IT_IT("PlanWindow::selectAndIdle");
		gotoIdle();
		selectObject(pEvent);
	};
	//
	void flush();
	//
	signals:
	//
	void ActiveObjectSelected(PlanActive *);
	void LinkSelected(PlanLink *);
	void PlanLoaded(const QString &); 
	void SelectActiveObject(const QString &);
	void RightClicked(const QString &);
	//
	public slots:
	//   
	//   Menu handlers   
	void Print(); 
	void setFillOn(); 
	void setFillOff();
	void setPen(); 
	void setFont();
	void setforeColour(); 
	void setbackColour(); 
	//
	void addLine();
	void addRectangle(); 
	void addIcon(); 
	void addLabel();
	void addCircle(); 
	void addActive(); 
	void addLink();
	//
	void newPlan();			// start a new map
	void Flash();
	public:
	friend class PlanTips; // tool tips class
};
class PlanTips : public QToolTip
{
	PlanWindow *pP;
	public:
	PlanTips(PlanWindow *parent) : QToolTip(parent),pP(parent) {};
	QRect FitRect(const QString &t, const QRect &r); 
	void maybeTip(const QPoint &pt);
};
#endif

