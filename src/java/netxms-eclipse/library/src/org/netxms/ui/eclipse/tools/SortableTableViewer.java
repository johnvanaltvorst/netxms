/**
 * 
 */
package org.netxms.ui.eclipse.tools;

import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;

/**
 * Implementation of TableViewer with column sorting support
 * 
 * @author victor
 *
 */
public class SortableTableViewer extends TableViewer
{
	public static final int DEFAULT_STYLE = -1;
	
	private TableColumn[] columns;
	private TableSortingListener sortingListener;
	
	/**
	 * Constructor
	 * 
	 * @param parent Parent composite for table control
	 * @param names Column names
	 * @param widths Column widths (may be null)
	 * @param defaultSortingColumn Index of default sorting column
	 */
	public SortableTableViewer(Composite parent, String[] names, int[] widths,
	                           int defaultSortingColumn, int defaultSortDir,
	                           int style)
	{
		super(new Table(parent, (style == DEFAULT_STYLE) ? (SWT.MULTI | SWT.FULL_SELECTION) : style));

		sortingListener = new TableSortingListener(this);
		
		columns = new TableColumn[names.length];
		for(int i = 0; i < names.length; i++)
		{
			columns[i] = new TableColumn(getTable(), SWT.LEFT);
			columns[i].setText(names[i]);
			if (widths != null)
				columns[i].setWidth(widths[i]);
			columns[i].setData("ID", new Integer(i));
			columns[i].addSelectionListener(sortingListener);
		}
		getTable().setLinesVisible(true);
		getTable().setHeaderVisible(true);

		getTable().setSortColumn(columns[defaultSortingColumn]);
		getTable().setSortDirection(defaultSortDir);
	}
}
