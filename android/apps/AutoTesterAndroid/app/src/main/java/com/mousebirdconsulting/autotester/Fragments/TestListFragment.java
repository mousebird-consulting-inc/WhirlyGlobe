package com.mousebirdconsulting.autotester.Fragments;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
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
import com.mousebirdconsulting.autotester.TestCases.*;

import org.jetbrains.annotations.NotNull;

import java.util.ArrayList;
import java.util.Objects;

public class TestListFragment extends Fragment {

	RecyclerView testList;

	private TestListAdapter adapter;
	private MaplyDownloadManager manager;

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		return inflater.inflate(R.layout.testlist_fragment, container, false);
	}

	@Override
	public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
		testList = view.findViewById(R.id.testList_recyclerList);
		adapter = new TestListAdapter();
		testList.setAdapter(adapter);
		testList.setLayoutManager(createLayoutManager());
	}

	@Override
	public void onDestroyView() {
		super.onDestroyView();
	}

	private RecyclerView.LayoutManager createLayoutManager() {
		return new LinearLayoutManager(Objects.requireNonNull(getActivity()).getApplicationContext());
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

		final private ArrayList<MaplyTestCase> testCases = new ArrayList<>();

		TestListAdapter() {
			Activity a = Objects.requireNonNull(getActivity());
			testCases.add(new StamenRemoteTestCase(a));
			testCases.add(new GeographyClass(a));
			testCases.add(new AnimatedBaseMapTestCase(a));
			testCases.add(new ImageReloadTestCase(a));
			testCases.add(new CustomBNGCoordAdapter(a));
			testCases.add(new CustomBNGTileSource(a));
			testCases.add(new ScreenLabelsTestCase(a));
			testCases.add(new ScreenMarkersTestCase(a));
			testCases.add(new MarkersTestCase(a));
			testCases.add(new AnimatedScreenMarkersTestCase(a));
			testCases.add(new AnimatedMarkersTestCase(a));
			testCases.add(new ClusteredMarkersTestCase(a));
			testCases.add(new MovingScreenMarkersTestCase(a));
			testCases.add(new VectorsTestCase(a));
			testCases.add(new GreatCircleTestCase(a));
			testCases.add(new SimpleStyleTestCase(a));
			testCases.add(new VectorStyleTestCase(a));
			testCases.add(new VectorHoleTestCase(a));
			testCases.add(new ShapefileTestCase(a));
			testCases.add(new WideVectorsTestCase(a));
			testCases.add(new TextureVectorTestCase(a));
			testCases.add(new SLDTestCase(a));
			testCases.add(new LoftedPolyTestCase(a));
			testCases.add(new StickersTestCase(a));
			testCases.add(new PagingLayerTestCase(a));
			testCases.add(new VectorMBTilesTestCase(a));
			testCases.add(new MapTilerTestCase(a));
			testCases.add(new MapTilerCircleTestCase(a));
			testCases.add(new OpenMapTilesHybridTestCase(a));
			testCases.add(new CartoTestCase(a));
			testCases.add(new ShapesTestCase(a));
//			testCases.add(new MaplyStarModelTestCase(a));
			testCases.add(new FindHeightTestCase(a));
//			testCases.add(new GestureFeedbackTestCase(a));
//			testCases.add(new LightingTestCase(a));
			testCases.add(new BillboardTestCase(a));
//			testCases.add(new CoordConversionTestCase(a));
//			testCases.add(new StartupShutdownTestCase(a));
//			testCases.add(new MarkersAndLinesTestCase(a));
//			testCases.add(new BoundsTestCase(a));
//			testCases.add(new LayerShutdownTestCase(a));
//			testCases.add(new GeomPointsTestCase(a));
//			testCases.add(new AutoRotateTestCase(a));
//			testCases.add(new ArealTestCase(a));
			testCases.add(new LocationTrackingRealTestCase(a));
			testCases.add(new LocationTrackingSimTestCase(a));
			testCases.add(new AnimationDelegateTestCase(a));
			testCases.add(new ComponentObjectLeakTestCase(a));
		}

		public void downloadResources() {
			ArrayList<MaplyTestCase> incompleteTest = new ArrayList<>();
			Context context = Objects.requireNonNull(getContext());
			for (MaplyTestCase testCase : this.testCases) {
				if (!testCase.areResourcesDownloaded()) {
					incompleteTest.add(testCase);
					ConfigOptions.setTestState(context, testCase.getTestName(), ConfigOptions.TestState.Downloading);
				} else {
					if (ConfigOptions.getTestState(context, testCase.getTestName()) != ConfigOptions.TestState.Selected) {
						ConfigOptions.setTestState(context, testCase.getTestName(), ConfigOptions.TestState.Ready);
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

		@NotNull
		@Override
		public RecyclerView.ViewHolder onCreateViewHolder(@NotNull ViewGroup parent, int viewType) {
			View view = LayoutInflater.from(getContext()).inflate(R.layout.testlistitemview, parent, false);
			return new TestViewHolder(view);
		}

		@Override
		public void onBindViewHolder(@NotNull RecyclerView.ViewHolder holder, int position) {
			((TestViewHolder) holder).bindViewHolder(testCases.get(position), position);
		}

		@Override
		public int getItemCount() {
			return testCases.size();
		}

		public void changeItemsState(boolean selected) {
			Context context = Objects.requireNonNull(getContext());
			for (MaplyTestCase testCase : testCases) {
				ConfigOptions.TestState state = selected ? ConfigOptions.TestState.Ready : ConfigOptions.TestState.Selected;
				ConfigOptions.setTestState(context, testCase.getTestName(), state);
			}
			notifyDataSetChanged();
		}


		public ArrayList<MaplyTestCase> getTestCases() {
			return testCases;
		}

		private class TestViewHolder extends RecyclerView.ViewHolder {

			final private TextView label;
			final private ImageView selected, map, globe, retry, download;
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
				//self = itemView;
			}

			public void bindViewHolder(final MaplyTestCase testCase, final int index) {
				//final private View self;
				this.index = index;

				this.label.setText(testCase.getTestName());
				final MainActivity activity = Objects.requireNonNull((MainActivity)getActivity());
				final Context context = Objects.requireNonNull(getContext());
				//if error
				switch (ConfigOptions.getTestState(context, testCase.getTestName())) {
					case Error:
						itemView.setBackgroundColor(ContextCompat.getColor(activity, R.color.colorRed));
						retry.setVisibility(View.VISIBLE);
						selected.setVisibility(View.INVISIBLE);
						map.setVisibility(View.INVISIBLE);
						globe.setVisibility(View.INVISIBLE);
						download.setVisibility(View.INVISIBLE);
						retry.setOnClickListener(v -> adapter.downloadTestResources(index));
						break;
					case Downloading:
						itemView.setBackgroundColor(ContextCompat.getColor(activity, R.color.colorGreen));
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
						itemView.setBackgroundColor(ContextCompat.getColor(activity, R.color.colorWhite));
						switch (ConfigOptions.getExecutionMode(context)) {
							case Multiple:
								changeItemState(ConfigOptions.getTestState(context,testCase.getTestName()) == ConfigOptions.TestState.Selected);
								map.setVisibility(View.INVISIBLE);
								globe.setVisibility(View.INVISIBLE);
								itemView.setOnClickListener(v -> {
									if (ConfigOptions.getTestState(context, testCase.getTestName()) == ConfigOptions.TestState.Ready){
										ConfigOptions.setTestState(context, testCase.getTestName(), ConfigOptions.TestState.Selected);
									} else {
										ConfigOptions.setTestState(context, testCase.getTestName(), ConfigOptions.TestState.Ready);
									}
									changeItemState(ConfigOptions.getTestState(context,testCase.getTestName()) == ConfigOptions.TestState.Selected);
									notifyItemChanged(index);
								});
								break;

							case Interactive:
								selected.setVisibility(View.INVISIBLE);
								if (testCase.getImplementation() == MaplyTestCase.TestExecutionImplementation.Both || testCase.getImplementation() == MaplyTestCase.TestExecutionImplementation.Map) {
									map.setVisibility(View.VISIBLE);
								}
								else {
									map.setVisibility(View.INVISIBLE);
								}
								if (testCase.getImplementation() == MaplyTestCase.TestExecutionImplementation.Both || testCase.getImplementation() == MaplyTestCase.TestExecutionImplementation.Globe) {
									globe.setVisibility(View.VISIBLE);
								}
								else {
									globe.setVisibility(View.INVISIBLE);
								}
								map.setOnClickListener(v -> {
									ConfigOptions.setTestType(getContext(), ConfigOptions.TestType.MapTest);
									if (!activity.isExecuting()) {
										activity.prepareTest(testCase);
										activity.runTest(testCase);
									}
								});
								globe.setOnClickListener(v -> {
									ConfigOptions.setTestType(getContext(), ConfigOptions.TestType.GlobeTest);
									if (!activity.isExecuting()) {
										activity.prepareTest(testCase);
										activity.runTest(testCase);
									}
								});
								break;

							case Single:
								selected.setVisibility(View.INVISIBLE);
								map.setVisibility(View.INVISIBLE);
								globe.setVisibility(View.INVISIBLE);
								itemView.setOnClickListener(v -> {
									if (!activity.isExecuting()) {
										activity.prepareTest(testCase);
										activity.runTest(testCase);
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
						switch (ConfigOptions.getExecutionMode(context)) {
							case Multiple:
								this.selected.setImageDrawable(getResources().getDrawable(R.drawable.ic_options_action));
								break;
							case Interactive:
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
