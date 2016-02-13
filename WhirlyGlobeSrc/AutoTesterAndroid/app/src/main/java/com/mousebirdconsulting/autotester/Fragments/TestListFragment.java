package com.mousebirdconsulting.autotester.Fragments;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.MainActivity;
import com.mousebirdconsulting.autotester.R;
import com.mousebirdconsulting.autotester.TestCases.AnimatedBaseMapTestCase;
import com.mousebirdconsulting.autotester.TestCases.GestureFeedbackTestCase;
import com.mousebirdconsulting.autotester.TestCases.MapBoxSatelliteTestCase;
import com.mousebirdconsulting.autotester.TestCases.ParticleSystemTestCase;
import com.mousebirdconsulting.autotester.TestCases.ScreenLabelsTestCase;
import com.mousebirdconsulting.autotester.TestCases.ScreenMarkersTestCase;
import com.mousebirdconsulting.autotester.TestCases.SimpleParticleSystemTestCase;
import com.mousebirdconsulting.autotester.TestCases.StamenRemoteTestCase;
import com.mousebirdconsulting.autotester.TestCases.StickersTestCase;
import com.mousebirdconsulting.autotester.TestCases.VectorsTestCase;

import java.util.ArrayList;

import butterknife.ButterKnife;
import butterknife.InjectView;


public class TestListFragment extends Fragment {
	@InjectView(R.id.testList_recyclerList)
	RecyclerView testList;

	private TestListAdapter adapter;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		return inflater.inflate(R.layout.testlist_fragment, container, false);
	}

	@Override
	public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
		ButterKnife.inject(this, view);
		adapter = new TestListAdapter();
		testList.setAdapter(adapter);
		testList.setLayoutManager(createLayoutManager());
	}

	private RecyclerView.LayoutManager createLayoutManager() {
		return new LinearLayoutManager(getActivity().getApplicationContext());
	}

	public void notifyIconChanged(int index) {
		this.adapter.notifyItemChanged(index);
	}

	public ArrayList<MaplyTestCase> getTests() {
		return adapter.getTestCases();
	}

	private class TestListAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

		private ArrayList<MaplyTestCase> testCases;

		TestListAdapter() {
			testCases = new ArrayList<>();
			testCases.add(new StamenRemoteTestCase(getActivity()));
			testCases.add(new MapBoxSatelliteTestCase(getActivity()));
			testCases.add(new AnimatedBaseMapTestCase(getActivity()));
			testCases.add(new VectorsTestCase(getActivity()));
			testCases.add(new ScreenLabelsTestCase(getActivity()));
			testCases.add(new ScreenMarkersTestCase(getActivity()));
			testCases.add(new StickersTestCase(getActivity()));
			testCases.add(new GestureFeedbackTestCase(getActivity()));
			testCases.add(new SimpleParticleSystemTestCase(getActivity()));
			testCases.add(new ParticleSystemTestCase(getActivity()));
		}

		@Override
		public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
			View view = LayoutInflater.from(getContext()).inflate(R.layout.testlistitemview, parent, false);
			return new TestViewHolder(view);
		}

		@Override
		public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
			((TestViewHolder) holder).bindViewHolder(testCases.get(position));
		}

		@Override
		public int getItemCount() {
			return testCases.size();
		}

		public ArrayList<MaplyTestCase> getTestCases() {
			return testCases;
		}

		private class TestViewHolder extends RecyclerView.ViewHolder {

			private TextView label;
			private ImageView selected;
			private View self;
			private MaplyTestCase testCase;

			public TestViewHolder(View itemView) {
				super(itemView);
				label = (TextView) itemView.findViewById(R.id.testNameLabel);
				selected = (ImageView) itemView.findViewById(R.id.itemSelected);
				self = itemView;
			}

			public void bindViewHolder(final MaplyTestCase testCase) {
				this.testCase = testCase;
				this.selected.setImageDrawable(getResources().getDrawable(testCase.getIcon()));
				changeItemState(testCase.isSelected());
				this.label.setText(this.testCase.getTestName());

				self.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						MainActivity activity = (MainActivity) getActivity();
						if (!activity.isExecuting()) {
							activity.prepareTest();
							activity.runTest(testCase);
						}
					}
				});
			}

			public void changeItemState(boolean setSelected) {
				this.selected.setVisibility(
					setSelected ? View.VISIBLE : View.INVISIBLE);
			}
		}
	}
}
