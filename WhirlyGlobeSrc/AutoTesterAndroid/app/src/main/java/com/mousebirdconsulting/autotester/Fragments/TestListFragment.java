package com.mousebirdconsulting.autotester.Fragments;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.mousebirdconsulting.autotester.ConfigOptions;
import com.mousebirdconsulting.autotester.Framework.MaplyDownloadManager;
import com.mousebirdconsulting.autotester.Framework.MaplyTestCase;
import com.mousebirdconsulting.autotester.MainActivity;
import com.mousebirdconsulting.autotester.R;
import com.mousebirdconsulting.autotester.TestCases.AnimatedBaseMapTestCase;
import com.mousebirdconsulting.autotester.TestCases.AnimatedScreenMarkersTestCase;
import com.mousebirdconsulting.autotester.TestCases.AtmosphereTestCase;
import com.mousebirdconsulting.autotester.TestCases.BillboardTestCase;
import com.mousebirdconsulting.autotester.TestCases.BoundsTestCase;
import com.mousebirdconsulting.autotester.TestCases.CartoDBMapTestCase;
import com.mousebirdconsulting.autotester.TestCases.ClusteredMarkersTestCase;
import com.mousebirdconsulting.autotester.TestCases.ComplexParticleSystemTestCase;
import com.mousebirdconsulting.autotester.TestCases.ComponentObjectLeakTestCase;
import com.mousebirdconsulting.autotester.TestCases.CoordConversionTestCase;
import com.mousebirdconsulting.autotester.TestCases.CustomBNGCoordAdapter;
import com.mousebirdconsulting.autotester.TestCases.CustomBNGTileSource;
import com.mousebirdconsulting.autotester.TestCases.GestureFeedbackTestCase;
import com.mousebirdconsulting.autotester.TestCases.LightingTestCase;
import com.mousebirdconsulting.autotester.TestCases.LocalVectorTileTestCase;
import com.mousebirdconsulting.autotester.TestCases.MBTilesImageTestCase;
import com.mousebirdconsulting.autotester.TestCases.MaplyStarModelTestCase;
import com.mousebirdconsulting.autotester.TestCases.MarkersAndLinesTestCase;
import com.mousebirdconsulting.autotester.TestCases.PagingLayerTestCase;
import com.mousebirdconsulting.autotester.TestCases.ParticleSystemTestCase;
import com.mousebirdconsulting.autotester.TestCases.ScreenLabelsTestCase;
import com.mousebirdconsulting.autotester.TestCases.ScreenMarkersTestCase;
import com.mousebirdconsulting.autotester.TestCases.ShapesTestCase;
import com.mousebirdconsulting.autotester.TestCases.SimpleParticleSystemTestCase;
import com.mousebirdconsulting.autotester.TestCases.StamenRemoteTestCase;
import com.mousebirdconsulting.autotester.TestCases.StartupShutdownTestCase;
import com.mousebirdconsulting.autotester.TestCases.StickersTestCase;
import com.mousebirdconsulting.autotester.TestCases.VectorsTestCase;

import java.util.ArrayList;

import butterknife.ButterKnife;
import butterknife.InjectView;


public class TestListFragment extends Fragment {

	@InjectView(R.id.testList_recyclerList)
	RecyclerView testList;

	private TestListAdapter adapter;
	private MaplyDownloadManager manager;

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

	public void changeItemsState(boolean selected) {
		adapter.changeItemsState(selected);
	}

	public void notifyIconChanged(int index) {
		this.adapter.notifyItemChanged(index);
	}

	public ArrayList<MaplyTestCase> getTests() {
		return adapter.getTestCases();
	}

	public void downloadResources() {
		if (adapter != null) {
			adapter.downloadResources();
		}
	}

	private class TestListAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

		private ArrayList<MaplyTestCase> testCases;

		TestListAdapter() {
			testCases = new ArrayList<>();
			testCases.add(new StamenRemoteTestCase(getActivity()));
			testCases.add(new CartoDBMapTestCase(getActivity()));
			testCases.add(new AnimatedBaseMapTestCase(getActivity()));
			testCases.add(new VectorsTestCase(getActivity()));
			testCases.add(new ScreenLabelsTestCase(getActivity()));
			testCases.add(new ScreenMarkersTestCase(getActivity()));
			testCases.add(new AnimatedScreenMarkersTestCase(getActivity()));
			testCases.add(new ClusteredMarkersTestCase(getActivity()));
			testCases.add(new StickersTestCase(getActivity()));
			testCases.add(new GestureFeedbackTestCase(getActivity()));
			testCases.add(new SimpleParticleSystemTestCase(getActivity()));
			testCases.add(new ParticleSystemTestCase(getActivity()));
			testCases.add(new ComplexParticleSystemTestCase(getActivity()));
			testCases.add(new CustomBNGTileSource(getActivity()));
			testCases.add(new CustomBNGCoordAdapter(getActivity()));
			testCases.add(new MaplyStarModelTestCase(getActivity()));
			testCases.add(new ComponentObjectLeakTestCase(getActivity()));
			testCases.add(new ShapesTestCase(getActivity()));
			testCases.add(new AtmosphereTestCase(getActivity()));
			testCases.add(new LightingTestCase(getActivity()));
			testCases.add(new MBTilesImageTestCase(getActivity()));
			testCases.add(new BillboardTestCase(getActivity()));
			testCases.add(new LocalVectorTileTestCase(getActivity()));
			testCases.add(new PagingLayerTestCase(getActivity()));
			testCases.add(new CoordConversionTestCase(getActivity()));
			testCases.add(new StartupShutdownTestCase(getActivity()));
			testCases.add(new MarkersAndLinesTestCase(getActivity()));
			testCases.add(new BoundsTestCase(getActivity()));
//			testCases.add(new ArealTestCase(getActivity()));
		}

		public void downloadResources() {
			ArrayList<MaplyTestCase> incompleteTest = new ArrayList<>();
			for (MaplyTestCase testCase : this.testCases) {
				if (!testCase.areResourcesDownloaded()) {
					incompleteTest.add(testCase);
					ConfigOptions.setTestState(getContext(), testCase.getTestName(), ConfigOptions.TestState.Downloading);
				} else {
					if (ConfigOptions.getTestState(getContext(), testCase.getTestName()) != ConfigOptions.TestState.Selected) {
						ConfigOptions.setTestState(getContext(), testCase.getTestName(), ConfigOptions.TestState.Ready);
					}
				}
				adapter.notifyDataSetChanged();
			}
			if (incompleteTest.size() > 0) {
				MaplyDownloadManager.MaplyDownloadManagerListener listener = new MaplyDownloadManager.MaplyDownloadManagerListener() {
					@Override
					public void onFinish() {
						adapter.notifyDataSetChanged();
					}

					@Override
					public void onTestFinished(MaplyTestCase testCase) {
						adapter.notifyDataSetChanged();
					}
				};
				manager = new MaplyDownloadManager(getActivity(), incompleteTest, listener);
				manager.execute();
			}
		}

		public void downloadTestResources(final int index) {
			if (index >= testCases.size()) {
				return;
			}
			ArrayList<MaplyTestCase> test = new ArrayList<>();
			test.add(testCases.get(index));
			MaplyDownloadManager.MaplyDownloadManagerListener listener = new MaplyDownloadManager.MaplyDownloadManagerListener() {
				@Override
				public void onFinish() {
					adapter.notifyItemChanged(index);
				}

				@Override
				public void onTestFinished(MaplyTestCase testCase) {
					adapter.notifyItemChanged(index);
				}
			};
			manager = new MaplyDownloadManager(getActivity(), test, listener);
			manager.execute();
		}

		@Override
		public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
			View view = LayoutInflater.from(getContext()).inflate(R.layout.testlistitemview, parent, false);
			return new TestViewHolder(view);
		}

		@Override
		public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
			((TestViewHolder) holder).bindViewHolder(testCases.get(position), position);
		}

		@Override
		public int getItemCount() {
			return testCases.size();
		}

		public void changeItemsState(boolean selected) {
			for (MaplyTestCase testCase : testCases) {
				ConfigOptions.TestState state = selected ? ConfigOptions.TestState.Ready : ConfigOptions.TestState.Selected;
				ConfigOptions.setTestState(getContext(), testCase.getTestName(), state);
			}
			notifyDataSetChanged();
		}


		public ArrayList<MaplyTestCase> getTestCases() {
			return testCases;
		}

		private class TestViewHolder extends RecyclerView.ViewHolder {

			private TextView label;
			private ImageView selected, map, globe, retry, download;
			private View self;
			private MaplyTestCase testCase;
			private int index;

			public int getIndex() {
				return index;
			}

			public TestViewHolder(View itemView) {
				super(itemView);
				label = (TextView) itemView.findViewById(R.id.testNameLabel);
				selected = (ImageView) itemView.findViewById(R.id.itemSelected);
				map = (ImageView) itemView.findViewById(R.id.map_icon);
				globe = (ImageView) itemView.findViewById(R.id.globe_icon);
				retry = (ImageView) itemView.findViewById(R.id.retryDownload);
				download = (ImageView) itemView.findViewById(R.id.downloading);
				self = itemView;
			}

			public void bindViewHolder(final MaplyTestCase testCase, final int index) {
				this.testCase = testCase;
				this.index = index;

				this.label.setText(this.testCase.getTestName());
				final MainActivity activity = (MainActivity) getActivity();
				//if error
				switch (ConfigOptions.getTestState(getContext(), testCase.getTestName())) {
					case Error:
						itemView.setBackgroundColor(ContextCompat.getColor(getActivity(), R.color.colorRed));
						retry.setVisibility(View.VISIBLE);
						selected.setVisibility(View.INVISIBLE);
						map.setVisibility(View.INVISIBLE);
						globe.setVisibility(View.INVISIBLE);
						download.setVisibility(View.INVISIBLE);
						retry.setOnClickListener(new View.OnClickListener() {
							@Override
							public void onClick(View v) {
								adapter.downloadTestResources(index);
							}
						});
						break;
					case Downloading:
						itemView.setBackgroundColor(ContextCompat.getColor(getActivity(), R.color.colorGreen));
						retry.setVisibility(View.INVISIBLE);
						selected.setVisibility(View.INVISIBLE);
						map.setVisibility(View.INVISIBLE);
						globe.setVisibility(View.INVISIBLE);
						download.setVisibility(View.VISIBLE);
						break;

					case Selected:
					case Ready:
						retry.setVisibility(View.INVISIBLE);
						download.setVisibility(View.INVISIBLE);
						itemView.setBackgroundColor(ContextCompat.getColor(getActivity(), R.color.colorWhite));
						switch (ConfigOptions.getExecutionMode(getContext())) {
							case Multiple:
								changeItemState(ConfigOptions.getTestState(getContext(),testCase.getTestName()) == ConfigOptions.TestState.Selected);
								map.setVisibility(View.INVISIBLE);
								globe.setVisibility(View.INVISIBLE);
								itemView.setOnClickListener(new View.OnClickListener() {
									@Override
									public void onClick(View v) {
										if (ConfigOptions.getTestState(getContext(), testCase.getTestName()) == ConfigOptions.TestState.Ready){
											ConfigOptions.setTestState(getContext(), testCase.getTestName(), ConfigOptions.TestState.Selected);
										} else {
											ConfigOptions.setTestState(getContext(), testCase.getTestName(), ConfigOptions.TestState.Ready);
										}
										changeItemState(ConfigOptions.getTestState(getContext(),testCase.getTestName()) == ConfigOptions.TestState.Selected);
										notifyItemChanged(index);
									}
								});
								break;

							case Interactive:
								selected.setVisibility(View.INVISIBLE);
								map.setVisibility(View.VISIBLE);
								globe.setVisibility(View.VISIBLE);
								map.setOnClickListener(new View.OnClickListener() {
									@Override
									public void onClick(View v) {
										ConfigOptions.setTestType(getContext(), ConfigOptions.TestType.MapTest);
										if (!activity.isExecuting()) {
											activity.prepareTest(testCase);
											activity.runTest(testCase);
										}
									}
								});
								globe.setOnClickListener(new View.OnClickListener() {
									@Override
									public void onClick(View v) {
										ConfigOptions.setTestType(getContext(), ConfigOptions.TestType.GlobeTest);
										if (!activity.isExecuting()) {
											activity.prepareTest(testCase);
											activity.runTest(testCase);
										}
									}
								});
								break;

							case Single:
								selected.setVisibility(View.INVISIBLE);
								map.setVisibility(View.INVISIBLE);
								globe.setVisibility(View.INVISIBLE);
								itemView.setOnClickListener(new View.OnClickListener() {
									@Override
									public void onClick(View v) {
										if (!activity.isExecuting()) {
											activity.prepareTest(testCase);
											activity.runTest(testCase);
										}
									}
								});
								break;
						}
						break;

					case Executing:
						selected.setVisibility(View.VISIBLE);
						map.setVisibility(View.INVISIBLE);
						globe.setVisibility(View.INVISIBLE);
						retry.setVisibility(View.INVISIBLE);
						download.setVisibility(View.INVISIBLE);
						switch (ConfigOptions.getExecutionMode(getContext())) {
							case Multiple:
								this.selected.setImageDrawable(getResources().getDrawable(R.drawable.ic_options_action));
								break;
							case Interactive:
								break;
							case Single:
								break;
						}
						break;

					case None:
						selected.setVisibility(View.INVISIBLE);
						map.setVisibility(View.INVISIBLE);
						globe.setVisibility(View.INVISIBLE);
						retry.setVisibility(View.INVISIBLE);
						download.setVisibility(View.INVISIBLE);
				}
			}
			public void changeItemState(boolean setSelected) {
				this.selected.setVisibility(
					setSelected ? View.VISIBLE : View.INVISIBLE);
			}
		}
	}
}
