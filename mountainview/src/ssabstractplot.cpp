#include "ssabstractplot.h"
#include <QPainter>

class SSAbstractPlotPrivate {
public:
	SSAbstractPlot *q;

	OverlayPainter *m_overlay_painter;
	OverlayPainter *m_underlay_painter;
	double m_xrange_min;
	double m_xrange_max;
	double m_yrange_min;
	double m_yrange_max;
	float m_vertical_zoom_factor;
	bool m_channel_flip;
	DiskReadMda m_timepoint_mapping;

	void do_paint(QPainter &painter,int W,int H);
};

SSAbstractPlot::SSAbstractPlot(QWidget *parent) : QWidget(parent)
{
	d=new SSAbstractPlotPrivate;
	d->q=this;

	d->m_underlay_painter=0;
	d->m_overlay_painter=0;
	d->m_xrange_min=0;
	d->m_xrange_max=0;
	d->m_yrange_min=0;
	d->m_yrange_max=0;
	d->m_vertical_zoom_factor=1.5; //changed on 7/29/15
	d->m_channel_flip=true;
}

SSAbstractPlot::~SSAbstractPlot()
{
	delete d;
}

Vec2 SSAbstractPlot::yRange() const
{
	return vec2(d->m_yrange_min,d->m_yrange_max);
}

Vec2 SSAbstractPlot::xRange() const
{
	return vec2(d->m_xrange_min,d->m_xrange_max);
}

float SSAbstractPlot::verticalZoomFactor()
{
	return d->m_vertical_zoom_factor;
}

void SSAbstractPlot::setXRange(const Vec2 &range)
{
	if ((range.x!=d->m_xrange_min)||(range.y!=d->m_xrange_max)) {
		d->m_xrange_min=range.x;
		d->m_xrange_max=range.y;
		emit xRangeChanged();
	}
}

void SSAbstractPlot::setYRange(const Vec2 &range)
{
	d->m_yrange_min=range.x;
	d->m_yrange_max=range.y;
}

void SSAbstractPlot::setUnderlayPainter(OverlayPainter *X)
{
	d->m_underlay_painter=X;
	update();
}

void SSAbstractPlot::setOverlayPainter(OverlayPainter *X)
{
	d->m_overlay_painter=X;
	update();
}

void SSAbstractPlot::setVerticalZoomFactor(float val)
{
	if (val==d->m_vertical_zoom_factor) return;
	d->m_vertical_zoom_factor=val;
	emit replotNeeded();
}

void SSAbstractPlot::paintEvent(QPaintEvent *evt)
{
	Q_UNUSED(evt)

	QPainter painter(this);

	d->do_paint(painter,width(),height());
}

bool SSAbstractPlot::channelFlip()
{
	return d->m_channel_flip;
}

void SSAbstractPlot::setTimepointMapping(const DiskReadMda &TM)
{
	d->m_timepoint_mapping=TM;
}

DiskReadMda SSAbstractPlot::timepointMapping()
{
	return d->m_timepoint_mapping;
}

void SSAbstractPlot::setChannelFlip(bool val)
{
	if (d->m_channel_flip==val) return;
	d->m_channel_flip=val;
	emit replotNeeded();
}


void SSAbstractPlotPrivate::do_paint(QPainter &painter, int W, int H)
{
	painter.fillRect(0,0,W,H,QColor(230,230,230));

	painter.setRenderHint( QPainter::Antialiasing );

	q->updateSize(W,H);

	q->paintPlot(&painter,W,H);

	if (m_underlay_painter) {
		m_underlay_painter->paint(&painter,W,H);
	}

	if (m_overlay_painter) {
		m_overlay_painter->paint(&painter,W,H);
	}
}
