/**
 * NetXMS - open source network management system
 * Copyright (C) 2003-2013 Victor Kirhenshtein
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
package org.netxms.ui.eclipse.dashboard.widgets;

import org.eclipse.swt.SWT;
import org.eclipse.ui.IViewPart;
import org.netxms.client.dashboards.DashboardElement;
import org.netxms.client.datacollection.GraphItem;
import org.netxms.ui.eclipse.charts.api.ChartColor;
import org.netxms.ui.eclipse.charts.api.ChartDciConfig;
import org.netxms.ui.eclipse.charts.api.ChartFactory;
import org.netxms.ui.eclipse.charts.api.Gauge;
import org.netxms.ui.eclipse.dashboard.widgets.internal.GaugeConfig;

/**
 * Dial chart element
 */
public class GaugeElement extends ComparisonChartElement
{
	private GaugeConfig config;
	
	/**
	 * @param parent
	 * @param data
	 */
	public GaugeElement(DashboardControl parent, DashboardElement element, IViewPart viewPart)
	{
		super(parent, element, viewPart);
		
		try
		{
			config = GaugeConfig.createFromXml(element.getData());
		}
		catch(Exception e)
		{
			e.printStackTrace();
			config = new GaugeConfig();
		}

		refreshInterval = config.getRefreshRate() * 1000;
		
		switch(config.getGaugeType())
		{
			case GaugeConfig.TEXT:
				chart = ChartFactory.createCurrentValueChart(this, SWT.NONE);
				break;
			default:
				chart = ChartFactory.createDialChart(this, SWT.NONE);
				break;
		}
		chart.setChartTitle(config.getTitle());
		chart.setTitleVisible(config.isShowTitle());
		chart.setLegendVisible(config.isShowLegend());
		((Gauge)chart).setLegendInside(config.isLegendInside());
		((Gauge)chart).setVertical(config.isVertical());
      ((Gauge)chart).setElementBordersVisible(config.isElementBordersVisible());
		((Gauge)chart).setMinValue(config.getMinValue());
		((Gauge)chart).setMaxValue(config.getMaxValue());
		((Gauge)chart).setLeftYellowZone(config.getLeftYellowZone());
		((Gauge)chart).setLeftRedZone(config.getLeftRedZone());
		((Gauge)chart).setRightYellowZone(config.getRightYellowZone());
		((Gauge)chart).setRightRedZone(config.getRightRedZone());
		((Gauge)chart).setFontName(config.getFontName());
		
		int index = 0;
		for(ChartDciConfig dci : config.getDciList())
		{
			chart.addParameter(new GraphItem(dci.nodeId, dci.dciId, 0, 0, Long.toString(dci.dciId), dci.getName()), 0.0);
			int color = dci.getColorAsInt();
			if (color != -1)
				chart.setPaletteEntry(index, new ChartColor(color));
			index++;
		}
		chart.initializationComplete();

		startRefreshTimer();
	}
	
	/* (non-Javadoc)
	 * @see org.netxms.ui.eclipse.dashboard.widgets.ComparisonChartElement#getDciList()
	 */
	@Override
	protected ChartDciConfig[] getDciList()
	{
		return config.getDciList();
	}
}
